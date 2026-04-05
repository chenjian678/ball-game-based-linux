#include "raylib.h"
class GameObject {
public:
    Vector2 position; 
    GameObject(Vector2 pos) : position(pos) {}
 
    virtual ~GameObject() {} 
};
class PhysicalObject : virtual public GameObject {
public:
    Vector2 velocity; 
    float radius;    
    PhysicalObject(Vector2 pos, Vector2 vel, float r) 
        : GameObject(pos), velocity(vel), radius(r) {}
};
class VisualObject : virtual public GameObject {
public:
    Color color;    
    bool visible;    
    VisualObject(Vector2 pos, Color c, bool vis) 
        : GameObject(pos), color(c), visible(vis) {}
};
class Ball : public PhysicalObject, public VisualObject {
public:
    int scoreValue;
    Ball(Vector2 pos, Vector2 vel, float r, Color c, bool vis, int score)
        : GameObject(pos), 
          PhysicalObject(pos, vel, r), 
          VisualObject(pos, c, vis), 
          scoreValue(score) {}
};
