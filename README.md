# Pyros-qt
A linux Qt front-end for [libPyros]
![2020-11-09-210744_1264x764_scrot](https://user-images.githubusercontent.com/73812529/98622807-f8a3ef80-22cf-11eb-9323-18f0830a3c6a.png)

## How to use
You can get started by importing some files into your database and adding tags to them, you can then search for a tag and it will bring up all files associated with that tag 
### Tag relations
You can make tags aliases of one another, that way if you search for one tag it will also search for all tags that it's been aliased with. You can also add tags as children of another tag, this is basically a like one-way alias, so if you search for a tag it will also search for all it's children, but it's children won't search for it (e.g. if you were to add "blue" as a child of "color", searching for "color" would show all files with a tag of "blue" or "color", but searching for "blue" would only show you results with "blue"). A tag's children can also have children this allows you to create a hierarchy of tags, there's also no limit to how many relaitons a tag can have.
### Filters
You can filter tags you don't want to see by puting a '-' infront of it, additionally there are a few special filters that allow you filter files by metadata
* limit:(number)

  Sets maximum number of results to return
* order:(order type)

  Set the order you want the results to return in you can choose to order by time, size, ext, mime, hash or set it to random (putting a '-' at the start will allow you to reverse the order)
* tagcount:(number)

  Only show files with x number of tags or you can put a < or > before the number to only show files with less than or greater than the number you entered
* mime:(mime/type)

  Only show files with a specified mime-type (e.g. mime:image/gif)
* hash:(hash)

  Only show file that's hash is an exact match
* ext:(extension)

  Only show files with a specified file extension (e.g. ext:gif)

You can also use [globbing] with tags, including the filters listed above.
## Dependencies
* Qt
* [libPyros]
* mpv (used for video and audio playback)
* ffmpegthumbnailer (optional; used for video thumbnails)
* zlib (optional; used for viewing compressed cbz/zip files)

[libPyros]: https://github.com/saelz/libPyros
[globbing]: https://en.wikipedia.org/wiki/Glob_(programming)
