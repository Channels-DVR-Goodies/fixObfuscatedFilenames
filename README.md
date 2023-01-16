# fixObfuscatedFilenames
looks for obfuscated filenames, and replaces them with the parent directory's name.

```
+-+- Some.Film.Title.(2048)
  |
  +- 897623afbc239efc.mkv
```

becomes

```
+-+- Some.Film.Title.(2048)
  |
  +- Some.Film.Title.(2048).mkv
```
