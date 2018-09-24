## Fbx import
  - Correct vertex attributes
  - Material import
  - Texture import (as embedded resources always)
## Editor
  - AnimationDriver export
  - Material editor
  - Animation editor
  - Refresh resource inspector on filesystem changes
  - Ability to open resource editors from resource inspector
  - Resource previews or icons in resource inspector
  - ? Local resource inspector
  
## General
  - BUG: Light data is still present in shaders when light sources get deleted from scene
  
  - Change on load object reference search so it will consider only loaded objects
  - Separate bind transform for skin mesh instead of current world transform 
  - Refine fbx skin data loading, investigate crashes on some files
  - Default diffuse, normal and specular maps (1pixel color)
  - ImGui Curve control
  - Timeline tool
  - Animation events
  - Submeshes, or subpolysets for different materials. Sort polygons by materials on fbx import
  - Create materials on fbx import
  - NormalMaps, SpecularMaps, shininess, tint, selfillumination
  - AmbientLight
  - AnimationDriver serialization
  - Collision system and components
  - Animation state system