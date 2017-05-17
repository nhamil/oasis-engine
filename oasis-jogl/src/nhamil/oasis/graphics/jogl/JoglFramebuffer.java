package nhamil.oasis.graphics.jogl;

import nhamil.oasis.core.GameLogger;
import nhamil.oasis.graphics.Framebuffer;
import nhamil.oasis.graphics.Texture;

public class JoglFramebuffer implements Framebuffer {

    private static final GameLogger log = new GameLogger(JoglFramebuffer.class);
    
    private JoglGlWrapper gl;
    private int width, height;
    private int id = 0;
    private JoglTexture color;
    private int depth;
    
    public JoglFramebuffer(int width, int height, boolean color, boolean depth, JoglGlWrapper gl) {
        id = gl.genFramebuffer();
        this.width = width;
        this.height = height;
        int oldId = gl.getBoundFbo();
        gl.bindFramebuffer(id);
        if (color) {
            this.color = new JoglTexture(width, height, gl);
            gl.framebufferTextureColor(this.color.getId());
        }
        if (depth) {
            this.depth = gl.genRenderbuffer();
            gl.bindRenderbuffer(this.depth);
            gl.renderbufferStorageDepth(width, height);
            gl.framebufferRenderbufferDepth(this.depth);
        }
        
        if (gl.checkFramebufferStatus()) {
            log.debug("Framebuffer good");
        }
        else {
            log.debug("Framebuffer bad");
        }
        
        gl.bindFramebuffer(oldId);
    }
    
    public int getId() {
        return id;
    }
    
    @Override
    public void dispose() {
        if (color != null) color.dispose();
        if (depth != 0) gl.deleteRenderbuffer(depth);
        gl.deleteFramebuffer(id);
        id = 0;
    }

    @Override
    public int getWidth() {
        return width;
    }

    @Override
    public int getHeight() {
        return height;
    }

    @Override
    public boolean hasTexture() {
        return color != null;
    }

    @Override
    public boolean hasDepthAttachment() {
        return depth != 0;
    }
    
    @Override
    public boolean hasStencilAttachment() {
        return false;
    }

    @Override
    public Texture getTexture() {
        return color;
    }

}