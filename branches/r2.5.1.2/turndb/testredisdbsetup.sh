#!/bin/sh

redis-cli <<!

select 0 
AUTH turn

set turn/user/ninefingers/key "bc807ee29df3c9ffa736523fb2c4e8ee"
set turn/user/gorst/key "7da2270ccfa49786e0115366d3a3d14d"

set turn/user/ninefingers/password "youhavetoberealistic"
set turn/user/gorst/password "hero"

set turn/secret/1368426581 "logen"
!
