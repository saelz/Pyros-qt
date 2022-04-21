# Pyros-qt
A linux Qt front-end for [libPyros]
![Preview](https://user-images.githubusercontent.com/73812529/98622807-f8a3ef80-22cf-11eb-9323-18f0830a3c6a.png)

## Getting started
You can get started by importing some files into your database and adding tags to them, you can then search for a tag which will bring up all files associated with that tag.

### Tag relationships
You can make tags aliases of one another, that way if you search for one tag it will also search for all tags that it's been aliased with. You can also add tags as children of another tag, this is basically like a one-way alias, so if you search for a tag it will also search for all it's children, but it's children won't search for it (e.g. if you were to add "blue" as a child of "color", searching for "color" would show all files with a tag of "blue" or "color", but searching for "blue" would only show you results with "blue"). A tag's children can also have children this allows you to create a hierarchy of tags, there is no limit to how many relationships can exist between tags.

### Filtering
You can filter tags you don't want to see by puting a `-` infront of it, additionally there are a few special filters that allow you filter files by metadata.

* `limit:(number)`
Sets the maximum number of results for search to return.

* `order:(order type)`
Sets the order of the search results, you can order the results by `time`, `size`, `ext`, `mime`, `hash` or set it to `random` (putting a `-` at the start will allow you to reverse the order).

* `tagcount:(number)`
Only show files with x number of tags or you can put a `<` or `>` before the number to only show files with less than or greater than the number you entered.

* `mime:(mime/type)`
Only show files with a specified mime-type e.g. `mime:image/gif`.

* `hash:(hash)`
Only show file that's hash is an exact match.

* `ext:(file extension)`
Only show files with a specified file extension e.g. `ext:gif`.

* `explicit:(tag)`
Only show files with this exact tag will ignore aliases and children.

You can also use [globbing] when searching for tags, this can also be used in conjunction with the aforementioned filters.

## Dependencies
* Qt
* [libPyros]
* `mpv`, used for video and audio playback
* `zlib`, used for viewing compressed cbz/zip files
* `ffmpeg` libraries: `libavcodec`, `libswscale`, `libavutil`, `libavformat`; used for video thumbnails

[libPyros]: https://github.com/saelz/libPyros
[globbing]: https://en.wikipedia.org/wiki/Glob_(programming)
