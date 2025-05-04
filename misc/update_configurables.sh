
OUTFILE=$1

shift
shift

cat >$OUTFILE <<EOF

#pragma target server

#include "Impl/Configure/ConfigureImpl.hpp"

namespace configure
{
EOF

configurable=$(grep -R "#pragma configurable" $@ | rev | cut -d' ' -f1 | rev)

for cfg in $configurable; do
    echo "    void $cfg(Hypodermic::ContainerBuilder &container);" >>$OUTFILE
done

cat >>$OUTFILE <<EOF
    void ConfigureImpl(Hypodermic::ContainerBuilder &container)
    {
EOF

for cfg in $configurable; do
    echo "        $cfg(container);" >>$OUTFILE
done

cat >>$OUTFILE <<EOF
    }
}
EOF
