cp -ar debian/changelog txt/

echo "GIT info:" > txt/git
git branch >> txt/git
echo "=================================" >> txt/git
echo "GIT history:" >> txt/git
git log -n 10 >> txt/git
