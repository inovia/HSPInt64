#runtime "hsp3_64"
#regcmd "hsp3cmdinit","hspint64.dll", 1
#cmd int64 $00
#cmd qpeek $01
#cmd qpoke $02
#cmd varptr64 $03
#cmd dupptr64 $04
#cmd callfunc64i $05
#cmd callfunc64d $06
#cmd callfunc64f $07