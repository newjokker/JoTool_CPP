# -*- coding: utf-8  -*-
# -*- author: jokker -*-

import os
import shutil


# 运行 ldd 可执行文件地址 > reqired.txt 
# python move_dll.py 即可将依赖的 so 保存到文件夹中
# 将 so 拷贝到环境下的 /usr/lib 即可加载

# ----------------------------------------------------------------------------
require_txt_path = r"required.txt"
save_dir = r"./so_dir"
# ----------------------------------------------------------------------------

os.makedirs(save_dir, exist_ok=True)

with open(require_txt_path, 'r') as txt_path:

    i = 1
    
    for each_line in txt_path:

        if i <= 1:
            i += 1
            continue

        each = each_line.strip()
        #
        if "=>" in each:
            each = each.split("=>")[1]

        each = each.split(" (")[0].strip()

        save_path = os.path.join(save_dir, os.path.split(each)[1])
        
        print('-'*30)
        print(each_line)
        print(each)
        print(save_path)
        
        
        shutil.copy(each, save_path)







