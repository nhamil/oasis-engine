package oasis.graphics;

import java.nio.Buffer;
import java.nio.ShortBuffer;

import oasis.core.Oasis;
import oasis.graphics.internal.InternalBuffer;

public class IndexBuffer {

    private ShortBuffer buffer; 
    private int size;
    private BufferUsage usage; 
    
    private boolean needsUpdate = true; 
    
    private InternalBuffer internal; 
    
    public IndexBuffer(int indices, BufferUsage usage) {
        this.size = indices; 
        this.usage = usage; 
        this.buffer = Oasis.getDirectBufferAllocator().allocate(size * 2).asShortBuffer(); 
        Oasis.getGraphicsDevice().linkInternal(this); 
    }
    
    void setInternal(InternalBuffer internal) {
        this.internal = internal; 
    }
    
    InternalBuffer getInternal() {
        return internal; 
    }

    public void release() {
        internal.release(); 
        needsUpdate = true; 
    }
    
    public void upload() {
        if (needsUpdate) {
            internal.uploadBuffer(); 
            needsUpdate = false; 
        }
    }
    
    public Buffer getAndFlipBuffer() {
        buffer.position(getSizeInShorts()); 
        buffer.flip(); 
        return buffer; 
    }
    
    public BufferUsage getUsage() {
        return usage; 
    }

    public int getSizeInBytes() {
        return size * 2; 
    }
    
    public int getSizeInShorts() {
        return size; 
    }
    
    public int getIndexCount() {
        return size; 
    }

    public void resize(int indices) {
        if (indices > buffer.capacity()) {
            Oasis.getDirectBufferAllocator().free(buffer); 
            buffer = Oasis.getDirectBufferAllocator().allocate(indices * 2).asShortBuffer(); 
        }
        size = indices; 
        buffer.limit(size); 
    }

    public void getIndices(int start, int count, short[] out, int outOffset) {
        buffer.position(start); 
        buffer.get(out, outOffset, count); 
    }

    public void setIndices(int start, int count, short[] in, int inOffset) {
        buffer.position(start); 
        buffer.put(in, inOffset, count); 
        
        needsUpdate = true; 
    }

}
