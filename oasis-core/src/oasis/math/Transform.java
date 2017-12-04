package oasis.math;

/**
 * 
 * 3D transform, has a position, rotation, and scale 
 * 
 * @author Nicholas Hamilton 
 *
 */
public class Transform {

    private Vector3 position; 
    private Quaternion rotation; 
    private Vector3 scale; 
    private Matrix4 matrix; 
    private boolean dirty = true; 
    
    public Transform() {
        position = new Vector3(); 
        rotation = new Quaternion(); 
        scale = new Vector3();
        matrix = Matrix4.identity(); 
    }
    
    public Vector3 getWorldPosition() { return position; }
    public Quaternion getWorldRotation() { return rotation; } 
    public Vector3 getWorldScale() { return scale; } 
    
    public Matrix4 getWorldMatrix() { 
        return getMatrix(); 
    }
    
    public Vector3 getPosition() { return position; } 
    public Quaternion getRotation() { return rotation; } 
    public Vector3 getScale() { return scale; } 
    
    public Matrix4 getMatrix() { 
        if (dirty) {
            matrix = Matrix4.identity(); 
            matrix.multiplySelf(Matrix4.translation(position)); 
            matrix.multiplySelf(Matrix4.rotation(rotation)); 
            matrix.multiplySelf(Matrix4.scale(scale)); 
        }
        return matrix; 
    }
    
    public void setPosition(Vector3 pos) { position.set(pos); } 
    public void setRotation(Quaternion rot) { rotation.set(rot); } 
    public void setScale(Vector3 scl) { scale.set(scl); } 
    
}
