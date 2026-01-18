set pagination off
set backtrace limit 0
set confirm off
set debuginfod enabled off
set print thread-events off

define hook-stop
  bt
end

run
