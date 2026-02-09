package com.jiemo.gimt_api

import org.junit.Test
import org.junit.Assert.*

/**
 * AIGCInfo 单元测试
 * 使用 TDD 思维，测试行为而非接口
 */
class AIGCInfoTest {

    /**
     * 测试行为：创建一个包含完整 AIGC 信息的对象
     */
    @Test
    fun shouldCreateAIGCInfoWithAllFields() {
        // Given: 准备完整的 AIGC 信息数据
        val label = "AIGC"
        val contentProducer = "OpenAI"
        val produceID = "gpt-4"
        val reservedCode1 = "reserved1"
        val contentPropagator = "JieMo"
        val propagateID = "jiemo-001"
        val reservedCode2 = "reserved2"

        // When: 创建 AIGCInfo 对象
        val aigcInfo = AIGCInfo(
            label = label,
            contentProducer = contentProducer,
            produceID = produceID,
            reservedCode1 = reservedCode1,
            contentPropagator = contentPropagator,
            propagateID = propagateID,
            reservedCode2 = reservedCode2
        )

        // Then: 验证对象包含正确的信息
        assertEquals(label, aigcInfo.label)
        assertEquals(contentProducer, aigcInfo.contentProducer)
        assertEquals(produceID, aigcInfo.produceID)
        assertEquals(reservedCode1, aigcInfo.reservedCode1)
        assertEquals(contentPropagator, aigcInfo.contentPropagator)
        assertEquals(propagateID, aigcInfo.propagateID)
        assertEquals(reservedCode2, aigcInfo.reservedCode2)
    }

    /**
     * 测试行为：创建一个空的 AIGC 信息对象
     */
    @Test
    fun shouldCreateEmptyAIGCInfo() {
        // When: 创建空的 AIGCInfo 对象
        val aigcInfo = AIGCInfo()

        // Then: 验证所有字段都是空字符串
        assertEquals("", aigcInfo.label)
        assertEquals("", aigcInfo.contentProducer)
        assertEquals("", aigcInfo.produceID)
        assertEquals("", aigcInfo.reservedCode1)
        assertEquals("", aigcInfo.contentPropagator)
        assertEquals("", aigcInfo.propagateID)
        assertEquals("", aigcInfo.reservedCode2)
    }

    /**
     * 测试行为：将 AIGC 信息序列化为 JSON 字符串
     */
    @Test
    fun shouldSerializeAIGCInfoToJson() {
        // Given: 创建一个包含数据的 AIGCInfo 对象
        val aigcInfo = AIGCInfo(
            label = "AIGC",
            contentProducer = "OpenAI",
            produceID = "gpt-4",
            reservedCode1 = "res1",
            contentPropagator = "JieMo",
            propagateID = "jiemo-001",
            reservedCode2 = "res2"
        )

        // When: 序列化为 JSON
        val json = aigcInfo.toJson()

        // Then: 验证 JSON 包含所有字段（使用 C++ 层相同的大写字段名）
        assertTrue(json.contains("\"Label\":\"AIGC\""))
        assertTrue(json.contains("\"ContentProducer\":\"OpenAI\""))
        assertTrue(json.contains("\"ProduceID\":\"gpt-4\""))
        assertTrue(json.contains("\"ReservedCode1\":\"res1\""))
        assertTrue(json.contains("\"ContentPropagator\":\"JieMo\""))
        assertTrue(json.contains("\"PropagateID\":\"jiemo-001\""))
        assertTrue(json.contains("\"ReservedCode2\":\"res2\""))
    }

    /**
     * 测试行为：从 JSON 字符串反序列化 AIGC 信息
     */
    @Test
    fun shouldDeserializeAIGCInfoFromJson() {
        // Given: 准备一个 JSON 字符串（使用 C++ 层相同的大写字段名）
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
        """.trimIndent()

        // When: 从 JSON 反序列化
        val aigcInfo = AIGCInfo.fromJson(json)

        // Then: 验证对象包含正确的信息
        assertEquals("AIGC", aigcInfo.label)
        assertEquals("OpenAI", aigcInfo.contentProducer)
        assertEquals("gpt-4", aigcInfo.produceID)
        assertEquals("res1", aigcInfo.reservedCode1)
        assertEquals("JieMo", aigcInfo.contentPropagator)
        assertEquals("jiemo-001", aigcInfo.propagateID)
        assertEquals("res2", aigcInfo.reservedCode2)
    }

    /**
     * 测试行为：序列化后再反序列化应该得到相同的数据（往返测试）
     */
    @Test
    fun shouldMaintainDataIntegrityAfterSerializationRoundtrip() {
        // Given: 创建原始对象
        val original = AIGCInfo(
            label = "AIGC",
            contentProducer = "OpenAI",
            produceID = "gpt-4",
            reservedCode1 = "reserved1",
            contentPropagator = "JieMo",
            propagateID = "jiemo-001",
            reservedCode2 = "reserved2"
        )

        // When: 序列化后再反序列化
        val json = original.toJson()
        val deserialized = AIGCInfo.fromJson(json)

        // Then: 验证数据完整性
        assertEquals(original.label, deserialized.label)
        assertEquals(original.contentProducer, deserialized.contentProducer)
        assertEquals(original.produceID, deserialized.produceID)
        assertEquals(original.reservedCode1, deserialized.reservedCode1)
        assertEquals(original.contentPropagator, deserialized.contentPropagator)
        assertEquals(original.propagateID, deserialized.propagateID)
        assertEquals(original.reservedCode2, deserialized.reservedCode2)
    }

    /**
     * 测试行为：处理包含特殊字符的 AIGC 信息
     */
    @Test
    fun shouldHandleSpecialCharactersInFields() {
        // Given: 创建包含特殊字符的对象
        val aigcInfo = AIGCInfo(
            label = "AIGC-测试",
            contentProducer = "OpenAI & Co.",
            produceID = "gpt-4.0",
            reservedCode1 = "res\"1\"",
            contentPropagator = "JieMo/Tech",
            propagateID = "jiemo_001",
            reservedCode2 = "res\\2"
        )

        // When: 序列化后再反序列化
        val json = aigcInfo.toJson()
        val deserialized = AIGCInfo.fromJson(json)

        // Then: 验证特殊字符被正确处理
        assertEquals(aigcInfo.label, deserialized.label)
        assertEquals(aigcInfo.contentProducer, deserialized.contentProducer)
        assertEquals(aigcInfo.produceID, deserialized.produceID)
        assertEquals(aigcInfo.reservedCode1, deserialized.reservedCode1)
        assertEquals(aigcInfo.contentPropagator, deserialized.contentPropagator)
        assertEquals(aigcInfo.propagateID, deserialized.propagateID)
        assertEquals(aigcInfo.reservedCode2, deserialized.reservedCode2)
    }

    /**
     * 测试行为：处理空字段的 JSON 序列化
     */
    @Test
    fun shouldSerializeEmptyFieldsCorrectly() {
        // Given: 创建一个空对象
        val aigcInfo = AIGCInfo()

        // When: 序列化为 JSON
        val json = aigcInfo.toJson()

        // Then: 验证 JSON 包含空字段
        assertTrue(json.contains("\"Label\":\"\""))
        assertTrue(json.contains("\"ContentProducer\":\"\""))
        assertTrue(json.contains("\"ProduceID\":\"\""))
        assertTrue(json.contains("\"ReservedCode1\":\"\""))
        assertTrue(json.contains("\"ContentPropagator\":\"\""))
        assertTrue(json.contains("\"PropagateID\":\"\""))
        assertTrue(json.contains("\"ReservedCode2\":\"\""))
    }

    /**
     * 测试行为：从不完整的 JSON 反序列化（缺少某些字段）
     */
    @Test
    fun shouldHandlePartialJsonGracefully() {
        // Given: 准备一个不完整的 JSON（只有部分字段）
        val json = """
            {
                "Label":"AIGC",
                "ContentProducer":"OpenAI"
            }
        """.trimIndent()

        // When: 从 JSON 反序列化
        val aigcInfo = AIGCInfo.fromJson(json)

        // Then: 验证存在的字段被正确解析，缺失的字段为空
        assertEquals("AIGC", aigcInfo.label)
        assertEquals("OpenAI", aigcInfo.contentProducer)
        assertEquals("", aigcInfo.produceID)
        assertEquals("", aigcInfo.reservedCode1)
        assertEquals("", aigcInfo.contentPropagator)
        assertEquals("", aigcInfo.propagateID)
        assertEquals("", aigcInfo.reservedCode2)
    }

    /**
     * 测试行为：两个相同内容的对象应该相等
     */
    @Test
    fun shouldConsiderTwoObjectsWithSameContentEqual() {
        // Given: 创建两个内容相同的对象
        val aigcInfo1 = AIGCInfo(
            label = "AIGC",
            contentProducer = "OpenAI",
            produceID = "gpt-4",
            reservedCode1 = "res1",
            contentPropagator = "JieMo",
            propagateID = "jiemo-001",
            reservedCode2 = "res2"
        )
        val aigcInfo2 = AIGCInfo(
            label = "AIGC",
            contentProducer = "OpenAI",
            produceID = "gpt-4",
            reservedCode1 = "res1",
            contentPropagator = "JieMo",
            propagateID = "jiemo-001",
            reservedCode2 = "res2"
        )

        // Then: 验证两个对象相等
        assertEquals(aigcInfo1, aigcInfo2)
        assertEquals(aigcInfo1.hashCode(), aigcInfo2.hashCode())
    }

    /**
     * 测试行为：两个不同内容的对象应该不相等
     */
    @Test
    fun shouldConsiderTwoObjectsWithDifferentContentNotEqual() {
        // Given: 创建两个内容不同的对象
        val aigcInfo1 = AIGCInfo(
            label = "AIGC",
            contentProducer = "OpenAI",
            produceID = "gpt-4",
            reservedCode1 = "res1",
            contentPropagator = "JieMo",
            propagateID = "jiemo-001",
            reservedCode2 = "res2"
        )
        val aigcInfo2 = AIGCInfo(
            label = "AIGC",
            contentProducer = "Google",
            produceID = "gemini",
            reservedCode1 = "res1",
            contentPropagator = "JieMo",
            propagateID = "jiemo-002",
            reservedCode2 = "res2"
        )

        // Then: 验证两个对象不相等
        assertNotEquals(aigcInfo1, aigcInfo2)
    }

    /**
     * 测试行为：复制对象应该创建一个独立的副本
     */
    @Test
    fun shouldCreateIndependentCopyWhenCopying() {
        // Given: 创建原始对象
        val original = AIGCInfo(
            label = "AIGC",
            contentProducer = "OpenAI",
            produceID = "gpt-4",
            reservedCode1 = "res1",
            contentPropagator = "JieMo",
            propagateID = "jiemo-001",
            reservedCode2 = "res2"
        )

        // When: 复制对象并修改副本
        val copy = original.copy(contentProducer = "Google", produceID = "gemini")

        // Then: 验证副本被修改，原始对象未受影响
        assertEquals("OpenAI", original.contentProducer)
        assertEquals("gpt-4", original.produceID)
        assertEquals("Google", copy.contentProducer)
        assertEquals("gemini", copy.produceID)
        // 其他字段应该相同
        assertEquals(original.label, copy.label)
        assertEquals(original.reservedCode1, copy.reservedCode1)
    }
}

