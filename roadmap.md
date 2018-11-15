## Fbx import
  - Material import
  - Texture import (as embedded resources always)
## Editor
  - Animation editor
  - Refresh resource inspector on filesystem changes
  
## General
  - Replace AnimationDriver with AnimationMixer supplied by XXXAnimSamplers

  - Change on load object reference search so it will consider only loaded objects
  - ImGui Curve control
  - Timeline tool
  - Animation events
  - Create materials on fbx import
  - NormalMaps
  - Collision system and components
  - Animation state system
  - Fix editor transform gizmos
  
## BUGS
  - Mesh resource causes crashes on draw if it doesn't have uvs or normals
  - BUG: Light data is still present in shaders when light sources get deleted from scene