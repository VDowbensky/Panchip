@echo off  
setlocal  
  
:: 假设你已经处于正确的Git仓库目录中  
  
:: 添加所有更改到暂存区  
git push -f --set-upstream origin master:master
