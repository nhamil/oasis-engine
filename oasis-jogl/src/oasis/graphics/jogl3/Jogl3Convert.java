package oasis.graphics.jogl3;

import java.util.HashMap;

import com.jogamp.opengl.GL;
import com.jogamp.opengl.GL2;

import oasis.graphics.Attribute;
import oasis.graphics.BufferUsage;
import oasis.graphics.Primitive;
import oasis.graphics.TextureFormat;

public class Jogl3Convert {
    private static final int[] PRIMITIVE_TYPE = new int[Primitive.values().length]; 
    private static final int[] BUFFER_USAGE = new int[BufferUsage.values().length]; 
    private static final int[] TEXTURE_FORMAT = new int[TextureFormat.values().length]; 
    private static final HashMap<String, Attribute> ATTRIBUTE_MAP = new HashMap<>(); 
    
    static { 
        PRIMITIVE_TYPE[Primitive.POINT_LIST.ordinal()] = GL.GL_POINTS; 
        PRIMITIVE_TYPE[Primitive.LINE_LIST.ordinal()] = GL.GL_LINES; 
        PRIMITIVE_TYPE[Primitive.LINE_STRIP.ordinal()] = GL.GL_LINES; 
        PRIMITIVE_TYPE[Primitive.TRIANGLE_LIST.ordinal()] = GL.GL_TRIANGLES; 
        PRIMITIVE_TYPE[Primitive.TRIANGLE_STRIP.ordinal()] = GL.GL_TRIANGLE_STRIP; 
        
        BUFFER_USAGE[BufferUsage.STATIC.ordinal()] = GL.GL_STATIC_DRAW; 
        BUFFER_USAGE[BufferUsage.DYNAMIC.ordinal()] = GL.GL_DYNAMIC_DRAW; 
        BUFFER_USAGE[BufferUsage.STREAM.ordinal()] = GL2.GL_STREAM_DRAW; 
        
        TEXTURE_FORMAT[TextureFormat.RGBA.ordinal()] = GL.GL_RGBA8; 
        TEXTURE_FORMAT[TextureFormat.RGBA8.ordinal()] = GL.GL_RGBA8; 
        TEXTURE_FORMAT[TextureFormat.RGBA16F.ordinal()] = GL.GL_RGBA16F; 
        TEXTURE_FORMAT[TextureFormat.RGBA32F.ordinal()] = GL.GL_RGBA32F; 
        TEXTURE_FORMAT[TextureFormat.DEPTH.ordinal()] = GL.GL_DEPTH_COMPONENT24; 
        TEXTURE_FORMAT[TextureFormat.DEPTH24.ordinal()] = GL.GL_DEPTH_COMPONENT24; 
        TEXTURE_FORMAT[TextureFormat.DEPTH32.ordinal()] = GL.GL_DEPTH_COMPONENT32; 
        
        for (Attribute a : Attribute.values()) { 
            ATTRIBUTE_MAP.put(a.getGlslName(), a); 
        }
    }
    
    public static int getPrimitiveInt(Primitive prim) { 
        return PRIMITIVE_TYPE[prim.ordinal()]; 
    }
    
    public static int getBufferUsageInt(BufferUsage usage) { 
        return BUFFER_USAGE[usage.ordinal()]; 
    }
    
    public static Attribute getAttributeFromString(String name) { 
        return ATTRIBUTE_MAP.get(name); 
    }
    
    public static int getTextureFormat(TextureFormat format) { 
        return TEXTURE_FORMAT[format.ordinal()]; 
    }
    
    public static int getDefaultTextureInputFormat(TextureFormat format) { 
        return format.isDepthFormat() ? GL2.GL_DEPTH_COMPONENT : GL.GL_RGBA; 
    }
    
    public static int getDefaultTextureInputType(TextureFormat format) { 
        return format.isDepthFormat() ? GL2.GL_UNSIGNED_INT : GL.GL_UNSIGNED_INT; 
    }
    
}