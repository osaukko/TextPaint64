#!/bin/sh
#Example: ./add_emblem.sh charset document-open
composite -gravity SouthWest "${1}_emblem_8x8.png" "../../../icons/16x16/${2}.png" "png32:../../../icons/16x16/${1}-${2}.png"
composite -gravity SouthWest "${1}_emblem_8x8.png" "../../../icons/22x22/${2}.png" "png32:../../../icons/22x22/${1}-${2}.png"
composite -gravity SouthWest "${1}_emblem_16x16.png" "../../../icons/32x32/${2}.png" "png32:../../../icons/32x32/${1}-${2}.png"
composite -gravity SouthWest "${1}_emblem_16x16.png" "../../../icons/48x48/${2}.png" "png32:../../../icons/48x48/${1}-${2}.png"

