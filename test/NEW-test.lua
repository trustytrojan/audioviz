package.cpath = package.cpath .. ';/home/t/Projects/audioviz/build/libaudioviz/?.so'
local audioviz = require('audioviz')

local function print_table(t)
	if type(t) ~= "table" then
		error("t is not a table")
	end

	for k, v in pairs(t) do
		print(k .. ": " .. tostring(v))
	end
end

print_table(audioviz)
