# OpenSSL依赖更新说明

## 更新内容

系统已更新为**强制要求**OpenSSL库支持，主要变更如下：

1. **移除标准库哈希备用实现**：
   - 删除了之前在无OpenSSL环境下使用C++标准库哈希函数的备用实现
   - 现在密码哈希完全依赖于OpenSSL的SHA-256算法

2. **CMakeLists.txt更新**：
   - 将OpenSSL从可选依赖改为必须依赖
   - 如果找不到OpenSSL库，构建将直接失败并提示错误信息
   - 移除了相关的条件编译宏定义

## 安装OpenSSL

不同系统下安装OpenSSL的方法：

### Linux
```bash
# Debian/Ubuntu
sudo apt-get install libssl-dev

# CentOS/RHEL
sudo yum install openssl-devel
```

### macOS
```bash
brew install openssl
```

### Windows
可通过以下方式获取：
1. 使用vcpkg：`vcpkg install openssl`
2. 直接下载预编译版本：https://slproweb.com/products/Win32OpenSSL.html
3. 使用MSYS2：`pacman -S mingw-w64-x86_64-openssl`

## 构建提示

如果构建失败并提示找不到OpenSSL：

1. 确保OpenSSL已正确安装
2. 使用CMake的`-DOPENSSL_ROOT_DIR`选项指定OpenSSL安装路径：
   ```bash
   cmake -DOPENSSL_ROOT_DIR=/path/to/openssl ..
   ```

## 安全性提升

此更新确保所有用户密码均使用加密安全的SHA-256算法进行哈希处理，提高了系统的整体安全性。 