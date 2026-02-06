# AIGCInfo API 文档

## 概述

`AIGCInfo` 是一个 Kotlin 数据类，用于表示 AIGC（AI Generated Content）元数据信息。该类与 C++ 层的 `AIGCInfo` 类保持一致，支持 JSON 序列化和反序列化。

## 类结构

### 字段说明

| 字段名 | JSON 字段名 | 类型 | 说明 |
|--------|-------------|------|------|
| `label` | `Label` | String | 标识 |
| `contentProducer` | `ContentProducer` | String | 内容生成者 |
| `produceID` | `ProduceID` | String | 生成者 ID |
| `reservedCode1` | `ReservedCode1` | String | 预留字段 1 |
| `contentPropagator` | `ContentPropagator` | String | 内容传播者 |
| `propagateID` | `PropagateID` | String | 传播者 ID |
| `reservedCode2` | `ReservedCode2` | String | 预留字段 2 |

### 主要方法

#### 构造函数
```kotlin
AIGCInfo(
    label: String = "",
    contentProducer: String = "",
    produceID: String = "",
    reservedCode1: String = "",
    contentPropagator: String = "",
    propagateID: String = "",
    reservedCode2: String = ""
)
```

#### toJson(): String
将 `AIGCInfo` 对象序列化为 JSON 字符串。

#### fromJson(json: String): AIGCInfo
从 JSON 字符串反序列化为 `AIGCInfo` 对象（静态方法）。

#### isEmpty(): Boolean
判断当前对象是否为空（所有字段都为空字符串）。

#### isValid(): Boolean
判断当前对象是否有效（至少有一个字段不为空）。

## 使用示例

### 创建对象

```kotlin
// 创建完整的 AIGCInfo 对象
val aigcInfo = AIGCInfo(
    label = "AIGC",
    contentProducer = "OpenAI",
    produceID = "gpt-4",
    reservedCode1 = "reserved1",
    contentPropagator = "JieMo",
    propagateID = "jiemo-001",
    reservedCode2 = "reserved2"
)

// 创建空对象
val emptyInfo = AIGCInfo()
```

### JSON 序列化

```kotlin
val aigcInfo = AIGCInfo(
    label = "AIGC",
    contentProducer = "OpenAI",
    produceID = "gpt-4"
)

val json = aigcInfo.toJson()
// 输出: {"Label":"AIGC","ContentProducer":"OpenAI","ProduceID":"gpt-4",...}
```

### JSON 反序列化

```kotlin
val json = """
{
    "Label":"AIGC",
    "ContentProducer":"OpenAI",
    "ProduceID":"gpt-4",
    "ReservedCode1":"res1",
    "ContentPropagator":"JieMo",
    "PropagateID":"jiemo-001",
    "ReservedCode2":"res2"
}
"""

val aigcInfo = AIGCInfo.fromJson(json)
println(aigcInfo.label) // 输出: AIGC
println(aigcInfo.contentProducer) // 输出: OpenAI
```

### 数据验证

```kotlin
val aigcInfo = AIGCInfo()
if (aigcInfo.isEmpty()) {
    println("AIGC 信息为空")
}

val validInfo = AIGCInfo(label = "AIGC")
if (validInfo.isValid()) {
    println("AIGC 信息有效")
}
```

### 对象复制

```kotlin
val original = AIGCInfo(
    label = "AIGC",
    contentProducer = "OpenAI",
    produceID = "gpt-4"
)

// 使用 data class 的 copy 方法
val modified = original.copy(
    contentProducer = "Google",
    produceID = "gemini"
)
```

## 测试覆盖

该类包含完整的单元测试，采用 TDD（测试驱动开发）方法，测试行为而非接口：

1. ✅ 创建包含完整信息的对象
2. ✅ 创建空对象
3. ✅ JSON 序列化
4. ✅ JSON 反序列化
5. ✅ 序列化/反序列化往返测试
6. ✅ 特殊字符处理
7. ✅ 空字段序列化
8. ✅ 不完整 JSON 处理
9. ✅ 对象相等性比较
10. ✅ 对象不等性比较
11. ✅ 对象复制独立性

所有测试用例均已通过。

## 依赖

- Gson 2.10.1（用于 JSON 序列化/反序列化）

## 注意事项

1. **字段命名约定**：JSON 字段名使用首字母大写的驼峰命名（如 `Label`、`ContentProducer`），与 C++ 层保持一致。
2. **默认值**：所有字段默认值为空字符串。
3. **异常处理**：`fromJson` 方法在解析失败时返回空对象，不会抛出异常。
4. **不可变性**：使用 `data class` 和 `val` 确保对象不可变，符合函数式编程最佳实践。
5. **线程安全**：由于对象不可变，因此天然线程安全。

## 与 C++ 层的兼容性

该类的设计与 C++ 层的 `AIGCInfo` 类完全兼容：

- 字段名称和类型一致
- JSON 格式相同
- 支持相同的序列化/反序列化操作
- 可以无缝与 C++ 层进行数据交换

