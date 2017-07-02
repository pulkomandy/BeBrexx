/* make buffer test */
queue "one"
queue "two"
queue "three"
say "Items in stack" queued()
call makebuf
say "New stack created" result
say "Items in stack" queued()
queue "deux"
queue "trois"
queue "quatre"
push "un"
do i=1 to queued()
	parse pull item
	say i item
end
say "Items in stack" queued()
call desbuf
say result "items where killed"

say "Items in stack" queued()
do i=1 to queued()
	parse pull item
	say i item
end
say "Items in stack" queued()
