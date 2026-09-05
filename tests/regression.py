#!/usr/bin/env python3
import os
import pathlib
import signal
import socket
import subprocess
import tempfile
import time


ROOT = pathlib.Path(__file__).resolve().parents[1]
SERVER = ROOT / "ircserv"
PASSWORD = "regression-secret-47"


def fail(message):
    raise AssertionError(message)


def free_port():
    probe = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    probe.bind(("127.0.0.1", 0))
    port = probe.getsockname()[1]
    probe.close()
    return port


def connect(port):
    client = socket.create_connection(("127.0.0.1", port), timeout=2.0)
    client.settimeout(0.15)
    return client


def receive_available(client, duration=0.8):
    chunks = []
    deadline = time.monotonic() + duration
    while time.monotonic() < deadline:
        try:
            chunk = client.recv(65536)
            if not chunk:
                break
            chunks.append(chunk)
        except socket.timeout:
            continue
    return b"".join(chunks)


def wait_for(client, marker, duration=2.0):
    data = b""
    deadline = time.monotonic() + duration
    while marker not in data and time.monotonic() < deadline:
        data += receive_available(client, 0.15)
    if marker not in data:
        fail("did not receive %r; received %r" % (marker, data[-500:]))
    return data


def register(client, nickname, fragmented=False):
    command = (
        "PASS %s\r\nNICK %s\r\nUSER %s 0 * :%s\r\n"
        % (PASSWORD, nickname, nickname, nickname)
    ).encode("ascii")
    if fragmented:
        cuts = (2, 9, 17, len(command) - 3)
        start = 0
        for end in cuts:
            client.sendall(command[start:end])
            start = end
            time.sleep(0.01)
        client.sendall(command[start:])
    else:
        client.sendall(command)
    wait_for(client, b"Welcome to the Internet Relay Network")


def assert_closed(client, duration=2.0):
    deadline = time.monotonic() + duration
    while time.monotonic() < deadline:
        try:
            if client.recv(4096) == b"":
                return
        except socket.timeout:
            continue
        except (ConnectionResetError, BrokenPipeError):
            return
    fail("connection remained open after an overlong IRC line")


def socket_flags(pid):
    fd_root = pathlib.Path("/proc") / str(pid) / "fd"
    info_root = pathlib.Path("/proc") / str(pid) / "fdinfo"
    result = []
    for entry in fd_root.iterdir():
        try:
            if not os.readlink(str(entry)).startswith("socket:"):
                continue
            fields = (info_root / entry.name).read_text().splitlines()
            flags_text = next(line.split()[1] for line in fields if line.startswith("flags:"))
            result.append(int(flags_text, 8))
        except (FileNotFoundError, PermissionError, StopIteration):
            continue
    return result


def wait_until(predicate, message, duration=2.0):
    deadline = time.monotonic() + duration
    while time.monotonic() < deadline:
        if predicate():
            return
        time.sleep(0.02)
    fail(message)


def can_connect(port):
    try:
        probe = socket.create_connection(("127.0.0.1", port), timeout=0.1)
        probe.close()
        return True
    except OSError:
        return False


def run_checks(proc, port):
    clients = []
    checks = []
    try:
        first = connect(port)
        clients.append(first)
        wait_until(
            lambda: len(socket_flags(proc.pid)) >= 2,
            "server did not expose both listening and accepted sockets",
        )
        flags = socket_flags(proc.pid)
        if not flags or any((value & os.O_NONBLOCK) == 0 for value in flags):
            fail("an accepted server socket is blocking")
        checks.append("accepted client sockets are O_NONBLOCK")

        register(first, "alpha", fragmented=True)
        checks.append("fragmented and coalesced IRC input is reassembled")

        no_user_params = connect(port)
        clients.append(no_user_params)
        no_user_params.sendall(
            ("PASS %s\r\nNICK noarg\r\nUSER\r\n" % PASSWORD).encode("ascii")
        )
        wait_for(no_user_params, b" 461 noarg USER :Not enough parameters")
        if proc.poll() is not None:
            fail("server exited after USER without parameters")
        no_user_params.sendall(b"USER noarg 0 * :noarg\r\n")
        wait_for(no_user_params, b"Welcome to the Internet Relay Network")
        checks.append("USER without parameters returns 461 and keeps the server alive")

        exact_prefix = b"NOTICE alpha :"
        exact_line = exact_prefix + (b"x" * (512 - len(exact_prefix) - 2)) + b"\r\n"
        if len(exact_line) != 512:
            fail("test setup did not create a 512-byte line")
        first.sendall(exact_line + b"LIST\r\n")
        wait_for(first, b"#lobby")

        overlong = connect(port)
        clients.append(overlong)
        register(overlong, "over")
        overlong.sendall(b"X" * 512)
        assert_closed(overlong)
        clients.remove(overlong)
        overlong.close()
        checks.append("512-byte boundary passes and unterminated overflow closes")

        first.sendall(b"NOTICE #channel-does-not-exist :still-alive\r\nLIST\r\n")
        wait_for(first, b"#lobby")
        if proc.poll() is not None:
            fail("server exited after NOTICE to a missing channel")
        checks.append("NOTICE to a missing channel does not dereference null")

        middle = connect(port)
        last = connect(port)
        clients.extend((middle, last))
        register(middle, "middle")
        register(last, "last")

        first.sendall(b"JOIN #invite\r\nMODE #invite +i\r\n")
        wait_for(first, b" MODE #invite +i")
        last.sendall(b"JOIN #invite\r\n")
        wait_for(last, b" 473 ")
        first.sendall(b"INVITE last #invite\r\n")
        wait_for(last, b" INVITE last :#invite")
        last.sendall(b"JOIN #invite\r\n")
        wait_for(last, b" JOIN #invite")
        checks.append("invite-only JOIN uses stable server user IDs")

        # A new channel contains its creator and the server bot.
        first.sendall(b"JOIN #limit-room\r\nMODE #limit-room +l 4\r\n")
        wait_for(first, b" MODE #limit-room +l 4")
        middle.sendall(b"JOIN #limit-room\r\n")
        wait_for(middle, b" JOIN #limit-room")
        last.sendall(b"JOIN #limit-room\r\n")
        wait_for(last, b" JOIN #limit-room")

        first.sendall(b"JOIN #limit-exact\r\nMODE #limit-exact +l 2\r\n")
        wait_for(first, b" MODE #limit-exact +l 2")
        last.sendall(b"JOIN #limit-exact\r\n")
        wait_for(last, b" 471 last #limit-exact :Cannot join channel (+l)")
        first.sendall(b"MODE #limit-exact -l\r\n")
        wait_for(first, b" MODE #limit-exact -l")
        last.sendall(b"JOIN #limit-exact\r\n")
        wait_for(last, b" JOIN #limit-exact")
        checks.append("channel user limit allows room and rejects the exact boundary")

        middle.close()
        clients.remove(middle)
        time.sleep(0.1)
        first.sendall(b"PRIVMSG last :after-hup\r\n")
        wait_for(last, b"after-hup")
        if proc.poll() is not None:
            fail("server exited while erasing a disconnected client")

        replacement = connect(port)
        clients.append(replacement)
        register(replacement, "middle")
        first.sendall(b"PRIVMSG middle :nickname-reused\r\n")
        wait_for(replacement, b"nickname-reused")
        checks.append("HUP removal preserves poll order and releases nickname state")

        no_newline = connect(port)
        clients.append(no_newline)
        no_newline.sendall(b"Y" * 512)
        assert_closed(no_newline)
        clients.remove(no_newline)
        no_newline.close()
        first.sendall(b"PRIVMSG last :after-overflow\r\n")
        wait_for(last, b"after-overflow")
        if proc.poll() is not None:
            fail("server exited while rejecting unterminated overflow")
        checks.append("pre-registration input buffer is bounded")

        return checks
    finally:
        for client in clients:
            try:
                client.close()
            except OSError:
                pass


def main():
    if not SERVER.exists():
        fail("ircserv is missing; run make first")

    port = free_port()
    with tempfile.TemporaryFile() as output:
        proc = subprocess.Popen(
            [str(SERVER), str(port), PASSWORD],
            cwd=str(ROOT),
            stdin=subprocess.PIPE,
            stdout=output,
            stderr=subprocess.STDOUT,
        )
        try:
            wait_until(
                lambda: proc.poll() is not None or can_connect(port),
                "server did not start listening",
            )
            if proc.poll() is not None:
                fail("server exited during startup")
            checks = run_checks(proc, port)
        finally:
            if proc.poll() is None and proc.stdin is not None:
                try:
                    proc.stdin.write(b"QUIT\n")
                    proc.stdin.flush()
                    proc.wait(timeout=2.0)
                except (BrokenPipeError, subprocess.TimeoutExpired):
                    proc.send_signal(signal.SIGTERM)
                    try:
                        socket.create_connection(("127.0.0.1", port), timeout=0.2).close()
                    except OSError:
                        pass
                    proc.wait(timeout=2.0)

        if proc.returncode != 0:
            fail("server exited with status %d" % proc.returncode)

        output.seek(0)
        log = output.read()
        if PASSWORD.encode("ascii") in log:
            fail("plaintext server password appeared in server logs")
        checks.append("plaintext server password is absent from logs")

    for check in checks:
        print("PASS " + check)
    print("PASS integration regression (%d checks)" % len(checks))


if __name__ == "__main__":
    main()
