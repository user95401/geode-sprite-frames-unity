# Sprite Frames Unity

<co>Developer Utility</c> for dynamically merging mod sprites into existing game textures.

## How it works
This mod hooks into Cocos2D's <cy>CCSpriteFrameCache::addSpriteFramesWithFile</c> to intercept when mods load their .plist files. When a mod tries to load sprites from a subdirectory (like <cg>mod_name/spritesheet.plist</c>), it:

1. **Detects the target texture** - Looks for the base game texture that should receive new sprites
2. **Creates a render canvas** - Uses <cy>CCRenderTexture</c> to composite the original texture with mod sprites  
3. **Merges textures in real-time** - Combines sprites while preserving original layout and quality
4. **Updates sprite frame coordinates** - Recalculates UV coordinates for the merged texture
5. **Replaces texture data** - Swaps the original texture with the merged version

## Technical Benefits
- **No file replacement** - Original game files remain untouched
- **Multiple mod support** - Multiple mods can add sprites to the same texture
- **Preserves quality** - Maintains texture parameters and formats

## For Mod Developers
Perfect for mods that need to add content to existing spritesheets like:
- Custom game objects in <cg>GJ_GameSheet</c> or <cg>GJ_GameSheet02</c>
- Additional UI elements in <cy>GJ_GameSheet03</c> (yea, there is batchnoded stuff too)

Add to your `mod.json` the `dependencies`:

```json
"dependencies": {
	"user95401.sprite-frames-unity": ">=v1.0.0"
}
```

[Live usage example on dialog trigger mod.](https://github.com/user95401/Dialog-Trigger)

<cr>Note:</c> <co>end users need mods that utilize this system to see changes.</c>