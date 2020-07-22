moduleName=$1
sed s/CreateLatVol/$moduleName/ _includes/modules/CreateLatVol.md > _includes/modules/$moduleName.md
ln -s ../_includes/modules/$moduleName.md modules/$moduleName.md
