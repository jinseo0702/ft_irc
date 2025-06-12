    message    =  [ ":" prefix SPACE ] command [ params ] crlf
    prefix     =  servername / ( nickname [ [ "!" user ] "@" host ] )
    Vcommand    =  1*letter
    params     =  *14( SPACE middle ) [ SPACE ":" trailing ]
               =/ 14( SPACE middle ) [ SPACE [ ":" ] trailing ]
    nospcrlfcl =  %x01-09 / %x0B-0C / %x0E-1F / %x21-39 / %x3B-FF
                    ; any octet except NUL, CR, LF, " " and ":"
    middle     =  nospcrlfcl *( ":" / nospcrlfcl )
    trailing   =  *( ":" / " " / nospcrlfcl )

    SPACE      =  %x20        ; space character
    crlf       =  %x0D %x0A   ; "carriage return" "linefeed"

---

    target     =  nickname / server
    msgtarget  =  msgto *( "," msgto )
    msgto     = channel / ( user "%" host ) / nickname /
            ( nickname "!" user "@" host )
    channel    =  ( "#" ) chanstring [ ":" chanstring ]
                ; 채널을 표현할 수있는 방법은 '#'으로 통일 하겠습니다.
    servername =  hostname
    host       =  hostname / hostaddr
    hostname   =  shortname *( "." shortname )
    shortname  =  ( letter / digit ) *( letter / digit / "-" )
                *( letter / digit )
                  ; as specified in RFC 1123 [HNAME] continuous hypen is failed;
    hostaddr   =  1*3digit "." 1*3digit "." 1*3digit "." 1*3digit
                ; ip4addr
    nickname   =  ( letter / special ) *8( letter / digit / special / "-" )
    chanstring =  %x01-07 / %x08-09 / %x0B-0C / %x0E-1F / %x21-2B
    chanstring =/ %x2D-39 / %x3B-FF
                  ; any octet except NUL, BELL, CR, LF, " ", "," and ":"
    
    Other parameter syntaxes are:

    user       =  1*( %x01-09 / %x0B-0C / %x0E-1F / %x21-3F / %x41-FF )
                  ; any octet except NUL, CR, LF, " " and "@"
    key        =  1*23( %x01-05 / %x07-08 / %x0C / %x0E-1F / %x21-7F )
                  ; any 7-bit US_ASCII character,
                    ; except NUL, CR, LF, FF, h/TABs, and " "
    letter     =  %x41-5A / %x61-7A       ; A-Z / a-z
    digit      =  %x30-39                 ; 0-9
    special    =  %x5B-60 / %x7B-7D
                   ; "[", "]", "\", "`", "_", "^", "{", "|", "}"