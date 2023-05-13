#!/bin/bash

read -p "Donnez la version que vous desirez: " version_number

read -p "Donnez le pourcentage de perte que vous voulez tolérer (marche qu'à partir de la v3): " loss_acceptability_number

file_path="./src/mictcp.c"

sed -i "s/version=[0-9]\+;/version=${version_number};/" "$file_path"

sed -i "s/#define LOSS_ACCEPTABILITY [0-9]\+/#define LOSS_ACCEPTABILITY ${loss_acceptability_number}/" "$file_path"

echo "Valeurs mises à jour avec success."
make
