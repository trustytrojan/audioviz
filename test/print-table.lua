---@diagnostic disable: lowercase-global, undefined-global
package.cpath = package.cpath .. ';/home/t/Projects/audioviz/build/libaudioviz/?.so'
audioviz = require('audioviz')

for k, v in pairs(audioviz) do
	print(k .. ": " .. tostring(v))
end
