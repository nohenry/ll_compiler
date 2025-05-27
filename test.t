89 45 0x45
def foo(int a) int
	return a * 2
end

foo 45

def search(#block block)
	items = [1, 2, 3, 4]
	i = 0
	while i < items.len
		return true if block(items[i])
	end
	return false
end

found = search do
	it == 23
end


