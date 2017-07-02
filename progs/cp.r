arg cmd args
select
   when cmd = 'SLEEP' then do
                         call time 'r'
                         do until time('e') > args
                         end
                      end
   otherwise nop;
end
