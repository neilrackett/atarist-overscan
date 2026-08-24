# Atari STE overscan

<video src="https://github.com/user-attachments/assets/1382505c-a8ce-404d-a8d7-5cc342749daf" width="640" height="480" autoplay loop muted controls></video>

Extend your Atari STE's pixels beyond their limits with these overscan examples
that being that extra screen space to life!

| Example              | Description                                                  |
| -------------------- | ------------------------------------------------------------ |
| `blitter`            | Bouncy Blitter Ball                                          |
| `hardware-scrolling` | Smooth scrolling all the way to the edges of your screen     |
| `any-st`             | Vertical overscan that works on any ST, not just the STE     |

The `blitter` and `hardware-scrolling` examples are STE-only and output 3 files:

- `NONE.TOS` as a baseline mode with no overscan for comparison (320x200)
- `VERTICAL.TOS` for top-bottom overscan (320x268)
- `FULL.TOS` for full-screen overscan (400x268)

The `any-st` example uses MFP timer interrupts instead of cycle-counted code,
so it runs on any ST model — STF, STFM, Mega ST, STE, and a Mega STE even at
16&nbsp;MHz with the cache enabled — and outputs 3 files:

- `TOP.TOS` removes the top border for up to 227 seamless lines (shown as
  320x224); the recommended variant for a game window
- `BOTTOM.TOS` removes the bottom border for up to 245 lines, with the caveat
  that picture line 200 always displays as border colour, so it suits games
  with a HUD or panel split at that height
- `BOTH.TOS` combines the two for 271 visible lines (320x271): 227 seamless
  lines, one line of border colour, then 44 more

## Build

You can build any of the examples by installing [atarist-toolkit-docker](https://github.com/sidecartridge/atarist-toolkit-docker) and running:

```bash
stcmd make
```

## Credits

Adapted from examples shared on [Atari-Forum](https://www.atari-forum.com/viewtopic.php?p=231136&sid=fb6439b4fd4d7a95dc81f13afed6924f#p231136).

## License

[GPL v3](LICENSE)
