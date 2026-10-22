# Issue #2 contribution scope

Base: StrangeDS/ViewFinderRe master 31e1ba0f7185ad95bd9abef30b566f2a0403b160.

Included: invalid-sender rewind termination fix; optional equipment, photo-container and pawn-transform-rewind components; camera-manager fallback and generic container lookup; Landscape conversion guard.

Excluded: placed-photo pickup/ReversePlace, nested Level Instance prefab rewrites, global asset-deletion handlers, forced primitive visibility, course maps/assets, View module references, project-specific configuration, and overrides of upstream object-pool fixes.

The three components live in the plugin ViewFinderRe module. Existing AVFCharacter is kept unchanged. Add the components to a non-AVFCharacter pawn, route input to the plugin camera/container actors as usual, and use VFEquipmentManagerComponent for exclusive equipment switching. Controllers may be assigned after BeginPlay. Rewind is single-player, as in upstream; this does not add multiplayer support.

Foliage-specific instance/frustum filtering is not claimed: this patch retains the upstream ISM backend without new changes.

Mac/UE 5.7 test adaptations are maintained in the ignored Validation directory, not in the upstream plugin descriptor. See the validation report for exact tested configuration and limits.
