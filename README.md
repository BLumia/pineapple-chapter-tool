Tool for adding chapter information to audio files.

*Heads up! Before this becomes a serious project, the code could be very messy and the program can be pretty buggy.*

## What's this?

Some people may have a collection of concert music files or audiobooks, which may contain several parts, or say, several chapters, combined as a single audio file. These audio files may not come with chapter marker data that can be useful for skipping around between different chapters of the audio, and for these cases, this tool comes to the rescue!

This tool can be used to add chapter information to audio files. MP3, OGG, and M4A formats are supported. Once the chapter data added to the audio file, you can then use them in audio players [1] and get the benefit of these chapter data.

[1]: Currently, seems only [mpv](https://mpv.io/) support all these formats mentioned above. [VLC](https://www.videolan.org/vlc/index.html) have [partial support of MP3(ID3v2) chapter frame](https://trac.videolan.org/vlc/ticket/7485) and I have plan to implement a fix and ensure it's upstreamable. My own music player will also plan to support chapter data in all mentioned formats.

## Technology Details?

 - For MP3 files, the ID3v2 tag will be used, chapter data will be added via the ID3v2 Chapter Frame.
 - For OGG files, Chapter Extension inside Xiph Vorbis Comment will be used.
 - For M4A files, chapters will be saved into a chapter track.

### For the players...

|                  | mp3          | ogg(vorbis/opus)    | m4a     |
| ---------------- | ------------ | ------------------- | ------- |
| mpv              | Yes          | Yes                 | Yes     |
| VLC (PC)         | Partial [^1] | Yes                 | Yes     |
| PotPlayer        | Yes [^2]     | Yes                 | Yes     |
| iTunes (Windows) | No           | No Playback Support | Yes[^3] |
| Movist (macOS)   | Yes          | No Playback Support | Yes     |
| MPC-HC           | Yes [^4]     | Yes                 | Yes     |

[^1]: VLC 3.x doesn't support it at all, VLC 4.x doesn't display MP3 chapter markers on timeline, some of MP3 files are not recognized ([BUG](https://trac.videolan.org/vlc/ticket/7485)).
[^2]: PotPlayer require MP3 file have at least one field not be empty in a general ID3v2 tag to display chapter frame correctly. 
[^3]: iTunes (Windows) doesn't display M4A chapter markers on timeline.
[^4]: The original MPC-HC is not, [this fork maintained by @clsid2](https://github.com/clsid2/mpc-hc) is supported.

## CLA

```
By sending patches in GitHub Pull Request, Issues, email patch 
set, or any other form to this project, it is assumed that you
are offering the Pineapple Chapter Tools project and the original
project author (Gary Wang) unlimited, non-exclusive right to
reuse, modify, and relicense the code.
```

This is important because the inability to relicense code has caused devastating problems for other Free Software projects (such as KDE and NASM). Pineapple Chapter Tool will always be available in an OSI approved, DFSG-compatible license. If you wish to specify special license conditions of your contributions, just say so when you send them.

## License

The source code of this project is licensed under [**GNU General Public License v2.0 only**](https://spdx.org/licenses/GPL-2.0-only.html) license. Individual files may have a different, but compatible license.
