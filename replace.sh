
 sed -i "s/::next::/::next::/g" `grep "::next::" -rl .`
 sed -i "s/next::/::next::/g" `grep "next::" -rl .`
 sed -i "s/namespace next/namespace next/g" `grep "namespace next" -rl .`

