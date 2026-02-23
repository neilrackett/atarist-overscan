# Atari STE overscan

<video src="https://github.com/user-attachments/assets/1382505c-a8ce-404d-a8d7-5cc342749daf" width="640" height="480" autoplay loop muted controls></video>

Extend your Atari STE's pixels beyond their limits with these overscan examples
that being that extra screen space to life!

| Example              | Description                                              |
| -------------------- | -------------------------------------------------------- |
| `blitter`            | Blitter powered bouncy ball demo                         |
| `hardware-scrolling` | Smooth scrolling all the way to the edges of your screen |

All of the examples output 3 files:

- `NONE.TOS` as a baseline mode with no overscan for comparison (320x200)
- `VERTICAL.TOS` for top-bottom overscan (320x268)
- `FULL.TOS` for full-screen overscan (400x268)

## Build

You can build any of the examples by installing [atarist-toolkit-docker](https://github.com/sidecartridge/atarist-toolkit-docker) and running:

```bash
stcmd make
```

## Credits

Adapted from examples shared on [Atari-Forum](https://www.atari-forum.com/viewtopic.php?p=231136&sid=fb6439b4fd4d7a95dc81f13afed6924f#p231136).

## License

[GPL v3](LICENSE)
