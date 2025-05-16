---@diagnostic disable: lowercase-global, undefined-global

-- if running off standalone lua executable
if not luaviz and package and package.cpath then
	package.cpath = package.cpath .. ';build/luaviz/?.so;build/luaviz/?.dll'
	luaviz = require('luaviz')
end

-- just make sure the table looks good
for k, v in pairs(luaviz) do
	print(k .. ': ' .. tostring(v))
end
