# Castle-wide stage resources

The on-demand catalog identifies 50 distinct US stage overlay/artwork pairs:
normal and inverted castle areas, the initial Castle Entrance, prologue, and
supported boss arenas. It uses original stage IDs and canonical file codes;
alternate/demo identities are not silently mapped to another stage.

`config/stages-us.json` contains paths, lengths, and SHA-1 revision fingerprints
only, checked against the pinned reconstruction's `config/check.us.sha` at
`7bc54d8157b6bed77e6bd15e5db9461a734fe1e5`. Maria's arena is excluded because its
artwork has no fingerprint in that list. These are compatibility identifiers,
not game bytes or authentication credentials.

`SotnStageAssets_Open` loads an independently verified overlay/artwork pair and
owns both until close. Failure releases partial initialization. Views borrow that
owner and remain stable if disk files later change. The existing bounded resource
loader checks regular files, exact size, EOF, and fingerprint. Code never executes
overlay machine instructions. No SDL graphics subsystem or eagerly loaded castle
is required. The existing five-file core snapshot remains compatible.

The headless inspector uses the same C stage/room parser as the game:

```sh
build/debug/bin/sotn-native --game-data /path/to/private/us --inspect-stage NO3
```

On macOS use the executable inside `sotn-native.app/Contents/MacOS/`. This is data
inspection, not a gameplay playtest. Public synthetic tests check identities,
lookup/path rejection, owned stable views, missing/corrupt artwork, partial failure,
and cleanup. A local audit verified loading and room parsing for all 50 pairs.
No private assets or asset-dependent results are required by public CI.

This removes the resource boundary's Laboratory-only assumption. It does **not**
implement native actors, scripts, audio, entry rules, or transitions for these
areas. The playable scene remains the selected Laboratory checkpoint. Stage-specific
behavior must be integrated and tested before a stage becomes a gameplay destination.
