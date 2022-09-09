# -*- coding: utf-8  -*-
# -*- author: jokker -*-

# -*- coding: utf-8  -*-
# -*- author: jokker -*-

import os
import shutil
import sys

# 运行 ldd 可执行文件地址 > reqired.txt
# python move_dll.py 即可将依赖的 so 保存到文件夹中
# 将 so 拷贝到环境下的 /usr/lib 即可加载

# todo 没必要分两步来做，直接 os.system("ldd ucd > ./require.txt") 生成需要的文件即可，一步直接得到所有的依赖文件

# 将所有的结果打成一个包，用这个脚本不同参数能直接安装就行
# 修改文件权限
# 将文件移动到对应的地方

def move_require_so_to_assign_dir(require_txt_path, so_dir):
    """将动态库移动到指定路径"""
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
            save_path = os.path.join(so_dir, os.path.split(each)[1])
            shutil.copy(each, save_path)

            print('-' * 30)
            print(each_line)
            print(each)
            print(save_path)

def get_require_txt(app_path, require_txt_path):
    os.system("ldd {0} > {1}".format(app_path, require_txt_path))



if __name__ == "__main__":

    if len(sys.argv) != 3:
        print("move_dll app_path save_dir")
    else:
        appPath = sys.argv[1]
        saveDir = sys.argv[2]

        # # ---------------------------
        # appPath = r"/usr/code/build/ucd"
        # saveDir = r"/usr/code/scripts/ucd_server"
        # # ---------------------------

        saveTxtPath = os.path.join(saveDir, "require.txt")
        soDir = os.path.join(saveDir, "so_dir")
        os.makedirs(saveDir)
        os.makedirs(soDir)

        get_require_txt(appPath, saveTxtPath)
        move_require_so_to_assign_dir(saveTxtPath, soDir)
        shutil.copy(appPath, os.path.join(saveDir, os.path.split(appPath)[1]))










