Parser를 더 필요한것만 단단하게 만들어보자

    message    =  [ ":" prefix SPACE ] command [ params ] crlf
    prefix     =  servername / ( nickname [ [ "!" user ] "@" host ] )
    command    =  1*letter
    params     =  *14( SPACE middle ) [ SPACE ":" trailing ]
               =/ 14( SPACE middle ) [ SPACE [ ":" ] trailing ]

    nospcrlfcl =  %x01-09 / %x0B-0C / %x0E-1F / %x21-39 / %x3B-FF
                    ; any octet except NUL, CR, LF, " " and ":"
    middle     =  nospcrlfcl *( ":" / nospcrlfcl )
    trailing   =  *( ":" / " " / nospcrlfcl )

    SPACE      =  %x20        ; space character
    crlf       =  %x0D %x0A   ; "carriage return" "linefeed"


  target     =  nickname / server
  msgtarget  =  msgto *( "," msgto )
  msgto      =  channel / ( user [ "%" host ] "@" servername )
  msgto      =/ ( user "%" host ) / targetmask
  msgto      =/ nickname / ( nickname "!" user "@" host )
  channel    =  ( "#" / "+" / ( "!" channelid ) / "&" ) chanstring
                [ ":" chanstring ]
  servername =  hostname
  host       =  hostname / hostaddr
  hostname   =  shortname *( "." shortname )
  shortname  =  ( letter / digit ) *( letter / digit / "-" )
                *( letter / digit )
                  ; as specified in RFC 1123 [HNAME]
  hostaddr   =  ip4addr / ip6addr
  ip4addr    =  1*3digit "." 1*3digit "." 1*3digit "." 1*3digit
  nickname   =  ( letter / special ) *8( letter / digit / special / "-" )
  targetmask =  ( "$" / "#" ) mask
                  ; see details on allowed masks in section 3.3.1
  chanstring =  %x01-07 / %x08-09 / %x0B-0C / %x0E-1F / %x21-2B
  chanstring =/ %x2D-39 / %x3B-FF
                  ; any octet except NUL, BELL, CR, LF, " ", "," and ":"
  channelid  = 5( %x41-5A / digit )   ; 5( A-Z / 0-9 )

  Other parameter syntaxes are:

  user       =  1*( %x01-09 / %x0B-0C / %x0E-1F / %x21-3F / %x41-FF )
                  ; any octet except NUL, CR, LF, " " and "@"
  key        =  1*23( %x01-05 / %x07-08 / %x0C / %x0E-1F / %x21-7F )
                  ; any 7-bit US_ASCII character,
                  ; except NUL, CR, LF, FF, h/v TABs, and " "
  letter     =  %x41-5A / %x61-7A       ; A-Z / a-z
  digit      =  %x30-39                 ; 0-9
  special    =  %x5B-60 / %x7B-7D
                   ; "[", "]", "\", "`", "_", "^", "{", "|", "}"






~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~``


    V message    =  [ ":" prefix SPACE ] command [ params ] crlf
    V prefix     =  servername / ( nickname [ [ "!" user ] "@" host ] )
    Vcommand    =  1*letter
    V params     =  *14( SPACE middle ) [ SPACE ":" trailing ]
               =/ 14( SPACE middle ) [ SPACE [ ":" ] trailing ]

    V nospcrlfcl =  %x01-09 / %x0B-0C / %x0E-1F / %x21-39 / %x3B-FF
                    ; any octet except NUL, CR, LF, " " and ":"
    V middle     =  nospcrlfcl *( ":" / nospcrlfcl )
    V trailing   =  *( ":" / " " / nospcrlfcl )

    V SPACE      =  %x20        ; space character
    V crlf       =  %x0D %x0A   ; "carriage return" "linefeed"


  V target     =  nickname / server
  msgtarget  =  msgto *( "," msgto )
  (
    msgto      =  channel / ( user [ "%" host ] "@" servername )
                ; msgto 로 보내는 target은 channel, ( user "%" host ), nickname, ( nickname "!" user "@" host ), 만 하겠습니다.
    msgto      =/ ( user "%" host ) / targetmask
    msgto      =/ nickname / ( nickname "!" user "@" host )
  )
  V msgto     = channel / ( user "%" host ) / nickname /
            ( nickname "!" user "@" host )
  V channel    =  ( "#" ) chanstring [ ":" chanstring ]
                ; 채널을 표현할 수있는 방법은 ( "#" / "+" / ( "!" channelid ) / "&" ) 아닌 '#'으로 통일 하겠습니다.
  V servername =  hostname
  V host       =  hostname / hostaddr
  V hostname   =  shortname *( "." shortname )
  V shortname  =  ( letter / digit ) *( letter / digit / "-" )
                *( letter / digit )
                  ; as specified in RFC 1123 [HNAME] continuous hypen is failed;
  V hostaddr   =  1*3digit "." 1*3digit "." 1*3digit "." 1*3digit
                ; ip4addr
  V nickname   =  ( letter / special ) *8( letter / digit / special / "-" )
  X targetmask =  ( "$" / "#" ) mask
                ; 사용하지 않겠습니다..
                  ; see details on allowed masks in section 3.3.1
  V chanstring =  %x01-07 / %x08-09 / %x0B-0C / %x0E-1F / %x21-2B
  V chanstring =/ %x2D-39 / %x3B-FF
                  ; any octet except NUL, BELL, CR, LF, " ", "," and ":"
  X channelid  = 5(digit)   
                ; 5(0-9) -> 채널아이디는 사용하지 않겠습니다.

  Other parameter syntaxes are:

  V user       =  1*( %x01-09 / %x0B-0C / %x0E-1F / %x21-3F / %x41-FF )
                  ; any octet except NUL, CR, LF, " " and "@"
  V key        =  1*23( %x01-05 / %x07-08 / %x0C / %x0E-1F / %x21-7F )
                  ; any 7-bit US_ASCII character,
                  ; except NUL, CR, LF, FF, h/v TABs, and " "
  V letter     =  %x41-5A / %x61-7A       ; A-Z / a-z
  V digit      =  %x30-39                 ; 0-9
  V special    =  %x5B-60 / %x7B-7D
                   ; "[", "]", "\", "`", "_", "^", "{", "|", "}"