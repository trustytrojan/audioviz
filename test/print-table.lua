package.cpath = package.cpath .. ';build/luaviz/?.so'
for k, v in pairs(require('luaviz')) do
	print(k .. ': ' .. tostring(v))
end
