(unwatch all)
(clear)
(dribble-on "Actual//focuscmd.out")
(batch "focuscmd.bat")
(dribble-off)
(clear)
(open "Results//focuscmd.rsl" focuscmd "w")
(load "compline.clp")
(printout focuscmd "focuscmd.bat differences are as follows:" crlf)
(compare-files "Expected//focuscmd.out" "Actual//focuscmd.out" focuscmd)
(close focuscmd)