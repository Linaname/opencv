package org.opencv.test.core;

import org.opencv.core.CvType;
import org.opencv.test.OpenCVTestCase;

public class CvTypeTest extends OpenCVTestCase {

    public void testMakeType() {
        assertEquals(CvType.CV_8UC4, CvType.makeType(CvType.CV_8U, 4));
    }

    public void testCV_8UC() {
        assertEquals(CvType.CV_8UC4, CvType.CV_8UC(4));
    }

    public void testCV_8SC() {
        assertEquals(CvType.CV_8SC4, CvType.CV_8SC(4));
    }

    public void testCV_16UC() {
        assertEquals(CvType.CV_16UC4, CvType.CV_16UC(4));
    }

    public void testCV_16SC() {
        assertEquals(CvType.CV_16SC4, CvType.CV_16SC(4));
    }

    public void testCV_32SC() {
        assertEquals(CvType.CV_32SC4, CvType.CV_32SC(4));
    }

    public void testCV_32FC() {
        assertEquals(CvType.CV_32FC4, CvType.CV_32FC(4));
    }

    public void testCV_64FC() {
        assertEquals(CvType.CV_64FC4, CvType.CV_64FC(4));
    }

    public void testCV_16FC() {
        assertEquals(CvType.CV_16FC1, CvType.CV_16FC(1));
        assertEquals(CvType.CV_16FC2, CvType.CV_16FC(2));
        assertEquals(CvType.CV_16FC3, CvType.CV_16FC(3));
        assertEquals(CvType.CV_16FC4, CvType.CV_16FC(4));
    }

    public void testChannels() {
        assertEquals(1, CvType.channels(CvType.CV_64F));
    }

    public void testDepth() {
        assertEquals(CvType.CV_64F, CvType.depth(CvType.CV_64FC3));
    }

    public void testIsInteger() {
        assertFalse(CvType.isInteger(CvType.CV_32FC3));
        assertTrue(CvType.isInteger(CvType.CV_16S));
    }

    public void testELEM_SIZE() {
        assertEquals(3 * 8, CvType.ELEM_SIZE(CvType.CV_64FC3));
        assertEquals(3 * 2, CvType.ELEM_SIZE(CvType.CV_16FC3));
    }

    public void testTypeToString() {
        assertEquals("CV_32FC1", CvType.typeToString(CvType.CV_32F));
        assertEquals("CV_32FC3", CvType.typeToString(CvType.CV_32FC3));
        assertEquals("CV_32FC(127)", CvType.typeToString(CvType.CV_32FC(127)));
    }

}
