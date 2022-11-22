mkdir('test-dir/move-to');
touch('test-dir/move-me.txt');
move('test-dir/move-me.txt', 'test-dir/move-to/move-me.txt');
