# Cows In Love

A small 3D game built to stress-test [eacp](https://github.com/eyalamirmusic/eacp),
our app framework: can we make awesome 3D games with eacp and ship them on every
serious platform? macOS, Windows, iOS and Android, through Steam and the app stores.

Find the other cow across the meadow and the ravine; when you do, the postcard's
kiss plays. `just macos` runs it; `just test` runs every library's tests; see
`CLAUDE.md` for the layout and `Deploy/README.md` for shipping.

## Dress Your Cow

The start menu's second choice dresses the cow: the camera swings round to her
and a row per item class ("Hat  <  Top Hat  >") sits beside her while she
stays live, and dragging (or the right stick) goes round her to see the hat
from every side. Left / right change the item, up / down move between rows,
and Done, Esc or the controller's East swings back to the menu. There are eight
hats: top hat, cowboy hat, party hat, beanie, crown, the frog hat, a cow-print
bucket hat with horns and ears, and a traffic cone; and
denim pants on both pairs of legs or on the back legs only.

Every change is saved at once to `CowSkin.json` in the app's support folder,
`~/Library/Application Support/Cows In Love/CowSkin.json` on macOS, as
enumerator names so it can be read and edited by hand:

```json
{
    "hat": "FrogHat",
    "pants": "BackLegs"
}
```

A missing or unreadable file, or a name the game does not know, starts her
bare. `COWS_DRESS=1` opens straight into the editor.
