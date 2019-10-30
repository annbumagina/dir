# dir
Searches for duplicated files in provided directory and subdirectories, and removes them if needed. The program has graphical interface based on the Qt Widgets library.

## Algorithm
1. Divide files by their size
2. In equal size group for each file compute Sha256 hash of first N bytes
3. In equal hash group for each file compute Sha256 hash of whole file
4. If hashes are equal then consider that files are equal
