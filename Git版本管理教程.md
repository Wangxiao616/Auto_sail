# Git 版本管理教程

## 目录

- [一、核心概念](#一核心概念)
- [二、本地仓库管理](#二本地仓库管理)
- [三、GitHub 远程仓库管理](#三github-远程仓库管理)
- [四、VSCode 源代码管理面板操作](#四vscode-源代码管理面板操作)
- [五、日常标准工作流程](#五日常标准工作流程)
- [附录一：常用命令速查表](#附录一常用命令速查表)
- [附录二：常见问题排查](#附录二常见问题排查)

---

## 一、核心概念

### Git 的四个区域

| 区域 | 说明 | 类比 |
|------|------|------|
| **工作区** | 你正在编辑的文件 | 你的桌面 |
| **暂存区** | `git add` 之后的临时区域 | 购物车 |
| **本地仓库** | `git commit` 后保存在 `.git` 里的版本历史 | 相册 |
| **远程仓库** | GitHub 上的副本 | 云端备份 |

### 数据流向

```
工作区  --git add-->  暂存区  --git commit-->  本地仓库  --git push-->  GitHub
          git restore <--        git reset <--              git pull <--
```

### 本地仓库与 GitHub 的关系

你电脑上执行完 `git init` 和 `git commit` 之后，版本历史全部存在 `.git` 文件夹里。GitHub 对此一无所知，需要手动关联和上传。

```
你电脑上的本地仓库                     GitHub 上的远程仓库
┌─────────────────────┐              ┌──────────────────────┐
│  .git/  版本历史     │   push  →   │  代码副本              │
│  源代码文件          │   ←  pull  │                      │
└─────────────────────┘              └──────────────────────┘
         ↑
   别名: origin  ←──────────────→  地址: https://github.com/你的用户名/仓库名.git
              (git remote add 建立这层映射)
```

---

## 二、本地仓库管理

### 第 1 步：初始化仓库

```bash
git init
```

在项目根目录执行，会生成 `.git` 隐藏文件夹，里面存着整个版本历史。一个项目只需执行一次。

---

### 第 2步：配置用户身份（仅首次，全局生效）

```bash
git config --global user.name "你的名字"
git config --global user.email "你的邮箱@example.com"
```

每次提交都会附上这个身份信息。只配一次，永久生效。

---

### 第 3 步：创建 .gitignore 文件

#### 它是什么？

`.gitignore` 是一个纯文本文件，告诉 Git："这些文件/文件夹，你当作没看见"。

- 文件名就叫 `.gitignore`，没有后缀（不像 `.txt`、`.c` 那样）
- 开头的点表示它是隐藏文件
- 放在项目根目录下，和 `.git` 文件夹同级

```
AUTO_SAIL_3/
├── .git/          ← Git 的版本库
├── .gitignore     ← 就放这里
├── User/
├── Core/
├── ...
```

#### 语法规则

| 写法 | 含义 |
|------|------|
| `file.txt` | 忽略根目录下的 file.txt |
| `*.exe` | 忽略所有 .exe 结尾的文件 |
| `build/` | 忽略 build 文件夹及其所有内容 |
| `/debug/` | 只忽略根目录下的 debug 文件夹 |
| `!重要.o` | 感叹号表示例外：不忽略"重要.o" |

- `*` 是通配符，`*.o` 表示"任意文件名，只要以 `.o` 结尾"
- 规则从上到下读取，命中任意一条即忽略
- 所有规则互相独立，顺序无所谓

#### STM32 项目示例

```
build/
*.o
*.obj
*.elf
*.hex
*.bin
*.map
*.list
.settings/
*.log
*.tmp
*.bak
.vscode/
*.swp
Thumbs.db
.DS_Store
```

#### 验证是否生效

```bash
git status
```

如果之前出现的文件现在消失了，说明 `.gitignore` 生效了。

注意：**已经在 Git 追踪中的文件，再加 .gitignore 是没用的**。`.gitignore` 只对还没有被 Git 追踪的新文件起作用。如果想把已在追踪的文件忽略：

```bash
git rm --cached 文件名    # 从 Git 追踪中移除，但不删除本地文件
```

---

### 第 4 步：第一次提交

**4.1 查看当前状态**

```bash
git status
```

所有文件列在 "Untracked files" 下面，表示 Git 还没追踪它们。

**4.2 将所有文件加入暂存区**

```bash
git add .
```

- `git add .`：把当前目录下所有文件加入暂存区
- 暂存区 = 购物车，先把要提交的文件放进去，然后一起结账

**4.3 执行提交**

```bash
git commit -m "初始提交：项目初始化"
```

- `-m`：后面跟提交信息（commit message）
- 这会在本地仓库创建一个版本快照，以后随时可以回退到这一刻

**4.4 确认提交完成**

```bash
git status
```

显示 "nothing to commit, working tree clean" 表示提交完毕。

---

### 第 5 步：日常修改与提交

```bash
# 1. 查看改了哪些文件
git status

# 2. 查看具体改了什么内容
git diff

# 3. 只暂存你改的那个文件
git add User/main.c

# 4. 提交
git commit -m "修改主循环逻辑"

# 5. 查看提交历史
git log --oneline
```

`git log --oneline` 输出示例：

```
a1b2c3d 修改主循环逻辑
e4f5g6h 初始提交：项目初始化
```

---

### 第 6 步：分支操作

**什么时候用分支？**
- 想尝试一个实验性功能，又不想破坏主代码
- 多人协作，各自在自己的分支上开发

**常用命令：**

```bash
# 创建新分支并切换过去（一步完成）
git checkout -b 新功能

# 或者分两步：
git branch 新功能      # 创建分支
git checkout 新功能    # 切换过去

# 在分支上做修改并提交...
git add .
git commit -m "添加新功能"

# 切回主分支
git checkout master

# 把新功能分支的内容合并到主分支
git merge 新功能

# 删除不再需要的分支
git branch -d 新功能
```

---

### 第 7 步：回退操作速查

| 场景 | 命令 |
|------|------|
| 改了文件但还没 `git add`，想放弃修改 | `git restore 文件名` |
| 已经 `git add` 了但还没 `commit`，想撤出暂存区 | `git restore --staged 文件名` |
| 已经 `commit` 了，想回退到上一个版本 | `git reset --soft HEAD~1` |

---

## 三、GitHub 远程仓库管理

### 第 1 步：确保有 GitHub 账号

没有的话去 [github.com](https://github.com) 注册。

建议在 VSCode 扩展商店安装 **GitHub Pull Requests** 插件，方便后续操作。

---

### 第 2 步：在 GitHub 网页上创建远程仓库

1. 浏览器打开 [github.com](https://github.com) 并登录
2. 点击右上角头像旁边的 **+** 号 → **New repository**
3. 填写：
   - **Repository name**：仓库名称（如 `AUTO_SAIL_3`）
   - **Description**：可选，描述这个项目
   - **Public**：公开 / **Private**：私有
4. **不要**勾选 "Add a README file"、"Add .gitignore"、"Choose a license"（本地已有代码）
5. 点击绿色的 **Create repository** 按钮
6. 创建后会显示类似以下内容，复制仓库地址（`https://github.com/你的用户名/AUTO_SAIL_3.git`）：

```
git remote add origin https://github.com/你的用户名/AUTO_SAIL_3.git
git branch -M main
git push -u origin main
```

---

### 第 3 步：将本地仓库关联到远程仓库（逐词详解）

#### 命令一：`git remote add origin https://github.com/...`

| 部分 | 含义 |
|------|------|
| `git remote` | 管理远程仓库的命令 |
| `add` | 添加一条远程仓库记录 |
| `origin` | 你给远程地址起的**别名**（约定俗成的叫法，可以改成任意名字） |
| `https://github.com/...` | 远程仓库的实际地址 |

**类比——手机通讯录：**

```
老妈   →  138-0000-0000
老板   →  139-1111-1111
origin →  https://github.com/你的用户名/AUTO_SAIL_3.git
```

之后 `git push` 或 `git pull` 时，Git 就知道跟 `origin` 打交道，不用每次都打长 URL。

执行：

```bash
git push -u origin main https://github.com/你的用户名/AUTO_SAIL_3.git
```

#### 命令二：验证

```bash
git remote -v

# 输出：
# origin  https://github.com/你的用户名/AUTO_SAIL_3.git (fetch)
# origin  https://github.com/你的用户名/AUTO_SAIL_3.git (push)
```

- `fetch`：从这个地址拉取代码
- `push`：向这个地址推送代码

如果什么也没显示，说明添加未成功。

#### 命令三：`git push -u origin master`

| 部分 | 含义 |
|------|------|
| `git push` | 把本地仓库的内容上传到远程仓库 |
| `origin` | 上传到哪个远程仓库（刚才添加的地址） |
| `master` | 上传哪个分支 |
| `-u` | 建立追踪关系，之后直接打 `git push` 就行 |

`-u`（upstream 的缩写）的作用：

```bash
# 没有 -u：每次都要写全
git push origin master

# 有 -u：Git 记住了默认对应关系
git push      # 自动知道往 origin 的 master 推
git pull      # 自动知道从 origin 的 master 拉
```

执行：

```bash
git push -u origin master
```

第一次推送可能会弹出浏览器让你登录 GitHub 授权。Windows 会记住凭据，以后不再弹。

推送成功后的输出示例：

```
Enumerating objects: 50, done.
Counting objects: 100% (50/50), done.
...
To https://github.com/Wangxiao/AUTO_SAIL_3.git
 * [new branch]      master -> master
Branch 'master' set up to track remote branch 'master' from 'origin'.
```

---

## 四、VSCode 源代码管理面板操作

### 打开面板

- 点击左侧边栏第三个图标（分支图标）
- 或者按 **Ctrl+Shift+G**

### 各操作对应鼠标/键盘方式

| 操作 | VSCode 面板操作 |
|------|----------------|
| **暂存单个文件** | 点击 "CHANGES" 区域文件名旁的 **+** 号 |
| **暂存所有文件** | 点击 "CHANGES" 标题旁的 **+** 号 |
| **提交** | 在顶部输入框写提交信息，按 **Ctrl+Enter** |
| **推送** | 点击底部状态栏 🔄 同步按钮，或 `...` → Push |
| **拉取** | 点击底部状态栏 🔄 同步按钮，或 `...` → Pull |
| **查看差异** | 点击 "CHANGES" 区域中任意修改过的文件 |
| **切换分支** | 点击底部状态栏最左侧的分支名，弹出列表选择 |
| **创建分支** | 点击底部状态栏分支名 → **+ Create new branch** |

### 差异视图说明

点击修改过的文件后，VSCode 打开左右对比视图：
- 左边：旧版本（HEAD，即上次提交的内容）
- 右边：你刚改的新版本
- 红色行：删除的内容
- 绿色行：新增的内容

---

## 五、日常标准工作流程

### 命令行版

```bash
git pull           # 1. 拉取 GitHub 上最新的代码
# ... 修改代码 ...
git add .          # 2. 暂存所有修改
git commit -m "描述你的改动"  # 3. 提交
git push           # 4. 推送到 GitHub
```

### VSCode 面板版

1. 点击 🔄 → 拉取
2. 修改代码
3. 在源代码管理面板点击 + 暂存
4. 输入提交信息，Ctrl+Enter 提交
5. 点击 🔄 → 推送

---

## 附录一：常用命令速查表

### 本地操作

| 操作 | 命令 |
|------|------|
| 初始化仓库 | `git init` |
| 查看状态 | `git status` |
| 暂存文件 | `git add 文件名` |
| 暂存所有 | `git add .` |
| 提交 | `git commit -m "提交信息"` |
| 查看历史（简洁） | `git log --oneline` |
| 查看差异 | `git diff` |
| 创建分支 | `git branch 分支名` |
| 切换分支 | `git checkout 分支名` |
| 创建并切换分支 | `git checkout -b 分支名` |
| 合并分支 | `git merge 分支名` |
| 删除分支 | `git branch -d 分支名` |
| 撤销工作区修改 | `git restore 文件名` |
| 撤销暂存 | `git restore --staged 文件名` |
| 回退一次提交 | `git reset --soft HEAD~1` |

### 远程操作

| 操作 | 命令 |
|------|------|
| 添加远程仓库 | `git remote add origin 地址` |
| 查看远程仓库 | `git remote -v` |
| 推送（首次） | `git push -u origin master` |
| 推送（日常） | `git push` |
| 拉取 | `git pull` |

---

## 附录二：常见问题排查

### 问题：`git push` 提示 "connection was reset" 或 "fatal: unable to access"

#### 第 1 步：定位原因

依次运行以下三条命令，确认问题出在哪个环节：

```bash
# 1. 检查远程仓库地址是否正确
git remote -v

# 2. 检查网络是否能连通 GitHub
ping github.com

# 3. 检查是否设置了代理（PowerShell 中把 grep 换成 Select-String）
git config --global --list
```

判断依据：

| 情况 | 原因 |
|------|------|
| `ping` 不通 | 网络问题，检查是否能上外网 |
| `ping` 通但 push 失败 | HTTPS 认证凭据问题（最常见） |
| `remote -v` 为空 | 还没关联远程仓库，回头看第三章第 3 步 |
| 配置中有 `http.proxy` | 代理干扰，运行 `git config --global --unset http.proxy` 清除 |

---

#### 第 2 步：根据原因解决

**情况一：认证凭据问题（`ping` 通但 push 失败）**

这是最常见的原因——Windows 保存的 GitHub 登录凭据过期或错误。

**解决方案 A：清除旧凭据**

1. 按 `Win` 键，搜索 **"凭据管理器"**（Credential Manager），打开它
2. 点击 **"Windows 凭据"**
3. 在"普通凭据"里找到所有以 `git:https://github.com` 开头的条目
4. 逐个点击 → **删除**
5. 关闭后回到终端重新 `git push`，会弹出浏览器让你登录

**解决方案 B：使用 Personal Access Token 手动认证**

如果浏览器登录窗口没有弹出，用 Token 代替密码：

1. 浏览器打开 [github.com/settings/tokens](https://github.com/settings/tokens)
2. 点击 **Generate new token (classic)**
3. 勾选 `repo`（完整仓库权限）
4. 点击生成，**复制那一串 token**（离开页面后就无法再看到）
5. 回终端执行：

```bash
git push -u origin master
```

- 用户名输入：你的 GitHub 用户名
- 密码输入：**粘贴那串 token**（不是你的 GitHub 登录密码）

**解决方案 C：切换为 SSH 协议（推荐，一劳永逸）**

改用 SSH 代替 HTTPS，彻底规避连接重置问题。

1. **生成 SSH 密钥**（终端执行，三次回车，全部默认）：
   ```bash
   ssh-keygen -t ed25519 -C "你的GitHub绑定邮箱"
   ```

2. **复制公钥**：打开 `C:\Users\你的用户名\.ssh\id_ed25519.pub`，复制全部内容

3. **绑定到 GitHub**：网页登录 GitHub → 右上角头像 → Settings → SSH and GPG keys → New SSH key → 粘贴公钥 → Add SSH key

4. **验证连通性**：
   ```bash
   ssh -T git@github.com
   ```
   出现 `Hi 用户名！You've successfully authenticated` 即成功（后半句 "GitHub does not provide shell access" 是正常提示，不影响使用）

5. **切换远程地址为 SSH 格式**：
   ```bash
   git remote set-url origin git@github.com:用户名/仓库名.git
   ```

6. **验证修改并推送**：
   ```bash
   git remote -v        # 确认地址已变为 git@github.com:... 格式
   git push -u origin main
   ```

---

**情况二：网络问题（`ping` 不通）**

- 检查是否能正常打开 [github.com](https://github.com)
- 如果无法访问，可能需要排查网络连接

---

**情况三：代理干扰**

```bash
# 清除 Git 代理设置
git config --global --unset http.proxy
git config --global --unset https.proxy
```

清除后重新尝试 `git push`。

---

#### 注意事项

- **PowerShell 中没有 `grep` 命令**，筛选输出用 `Select-String`，例如：
  ```powershell
  git config --global --list | Select-String proxy
  ```
- 每次解决完问题后，建议用 `git push -u origin master`（带 `-u`）而非 `git push`，确保追踪关系建立成功
