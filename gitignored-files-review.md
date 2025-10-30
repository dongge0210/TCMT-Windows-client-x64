# Git 忽略文件审查报告

## 概述
本文档记录了被 `.gitignore` 忽略但可能包含有价值信息的文件和目录。

**重要提醒**: 这些被 git 忽略的文件通常占用大量 tokens，如果确实需要读取，请通过终端手段（如 `cat`, `ls`, `find` 等命令）选择性查看，而不是直接使用文件读取工具。

## 重要发现

### 1. .iflow/ 目录 (被忽略)
**路径**: `.iflow/`
**状态**: ✅ **重要 - 应该关注**
**内容**: 
- `settings.json` - iFlow CLI 配置文件，包含 MCP 服务器配置

**价值**: 
- 包含代码运行器和任务管理器的配置
- 对开发环境设置有参考价值
- 建议考虑添加到版本控制或创建配置模板

### 2. .litho/ 目录 (被忽略)
**路径**: `.litho/`
**状态**: ⚠️ **中等关注**
**内容**:
- `cache/` - 包含大量缓存数据
  - `ai_code_insight/` - AI 代码洞察缓存
  - `ai_code_purpose/` - AI 代码用途分析
  - `documentation/` - 文档缓存
  - `studies_research/` - 研究缓存
  - 其他 AI 相关缓存目录

**价值**:
- 包含 AI 分析的缓存数据，可能包含有用的分析结果
- 文件较大，占用存储空间
- 建议定期清理，或提取有价值的结果到正式文档

### 3. 构建输出目录 (被忽略)
**路径**: `Project1/x64/Debug/`
**状态**: ⚠️ **构建相关**
**内容**:
- `Project1.Build.CppClean.log` - 构建清理日志
- `Project1.exe.recipe` - 可执行文件构建配方
- `Project1.tlog/` - 构建时间日志
- `Project1.vcxproj.FileListAbsolute.txt` - 项目文件列表

**价值**:
- 构建调试信息，对解决构建问题有帮助
- 通常可以忽略，但在构建失败时有诊断价值

### 4. 日志文件 (被忽略)
**路径**: `./Project1/x64/Debug/Project1.Build.CppClean.log`
**状态**: 📝 **调试信息**
**内容**: 构建清理日志（当前为空文件）

**价值**:
- 构建过程的调试信息
- 仅在构建问题时有参考价值

## 其他被忽略的文件类型

### 系统文件
- `.DS_Store` - macOS 系统文件
- `._*` - macOS 资源文件
- **建议**: 继续忽略，无开发价值

### 构建产物
- `*.exe`, `*.dll`, `*.lib` - 编译输出
- `obj/`, `bin/` - 构建目录
- **建议**: 继续忽略，应该重新构建

### IDE 配置
- `.vs/`, `*.suo`, `*.user` - Visual Studio 配置
- `.vscode/` - VS Code 配置
- **建议**: 继续忽略，个人开发环境配置

## 建议行动

### 高优先级
1. **保留 `.iflow/settings.json` 的配置模板**
   - 创建 `iflow.settings.template.json`
   - 添加到文档中说明配置选项

### 中优先级
2. **定期清理 `.litho/cache/`**
   - 设置缓存大小限制
   - 提取有价值的分析结果到正式文档

3. **构建失败时保留日志**
   - 在 CI/CD 流程中保存构建日志
   - 本地开发时可选择性保留

### 低优先级
4. **文档化忽略策略**
   - 在项目文档中说明忽略策略
   - 为新开发者提供指导

## iFlow CLI 相关工具

### Spec-Kit 集成
Spec-Kit 是 GitHub 开发的规范驱动开发工具，可与 iFlow CLI 结合使用：

#### 核心命令
- `/specify` - 定义项目规范和需求
- `/plan` - 基于规范生成实施计划  
- `/tasks` - 查看和执行具体任务

#### MCP 安装方式
```bash
npx -y mcp-server-spec-driven-development@latest
```

#### 与 iFlow CLI 的配合使用
1. 使用 iFlow CLI 管理 MCP 服务器
2. 通过 Spec-Kit 规范化开发流程
3. 结合 litho 文档分析项目结构

## 更新日期
2025-10-30

## 审查者
iFlow CLI