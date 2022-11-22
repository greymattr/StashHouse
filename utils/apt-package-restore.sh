#!/bin/bash

echo "collecting packages to be-reinstalled from /var/log/apt/history.log"
echo '#!/bin/bash' > restore
echo sudo apt-get install `grep Remove /var/log/apt/history.log | tail -1 | sed -e 's|Remove: ||g' -e 's|([^)]*)||g' -e 's|:[^ ]* ||g' -e 's|,||g'` >> restore
chmod +x restore

echo "DONE"
echo "now run ./restore"