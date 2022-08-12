# ucd 使用文档

### 目的

* 数据 与 开发之间解耦

* 数据和开发之间增加一个服务层

* 数据之间的传递使用 json 进行代替


### 实现方式

* 111 机器上提供 文件服务，开发人员与文件服务交互，数据人员与文件服务和源文件交互，开发人员不与数据人员交互

* 提供与 文件服务交互的 python 包，在 JoTools 中的 txkj/UCDatasetUtil 模块

* 提供与 文件服务交互的 应用程序 ucd(uc dataset operate)，便捷安装和使用 


### todo

* nginx 增加服务的速度和稳定性，同时分布在多个服务器上提供对应的服务


### 常用命令


* ucd save json_path save_dir save_mode(111|100) {need_count}
* 
* ucd check
* 
* ucd show
* 
* ucd delete ucd_name
* 
* ucd load ucd_name save_path|save_dir 
* 
* ucd upload ucd_path {ucd_name}
* 
* ucd from img_dir ucd_save_path
* 
* ucd info ucd_path






