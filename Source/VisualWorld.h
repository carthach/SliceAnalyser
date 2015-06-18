/*
* Copyright (c) 2006-2009 Erin Catto http://www.box2d.org
*
* This software is provided 'as-is', without any express or implied
* warranty.  In no event will the authors be held liable for any damages
* arising from the use of this software.
* Permission is granted to anyone to use this software for any purpose,
* including commercial applications, and to alter it and redistribute it
* freely, subject to the following restrictions:
* 1. The origin of this software must not be misrepresented; you must not
* claim that you wrote the original software. If you use this software
* in a product, an acknowledgment in the product documentation would be
* appreciated but is not required.
* 2. Altered source versions must be plainly marked as such, and must not be
* misrepresented as being the original software.
* 3. This notice may not be removed or altered from any source distribution.
*/

#ifndef VISUAL_WORLD_H
#define VISUAL_WORLD_H

class VisualWorld : public Test
{
public:
    VisualWorld()
    {
        m_world->SetGravity(b2Vec2(0.0f, 0.0f));
        
        int numBodies = 5;
        
        Random random;
        
        
        b2Body* m_ball;
        
        for ( int i = 0; i < numBodies; i++ )
        {
            b2BodyDef bd;
            
            float xMeters = 0.02f * 1024.0;
            float yMeters = 0.02f * 768.0;
            
            bd.position.Set(xMeters, yMeters);
            bd.type = b2_dynamicBody;
            bd.bullet = true;
            
            bodies[i] = m_world->CreateBody(&bd);
            
            b2CircleShape shape;
            shape.m_radius = 0.2f;
            
            b2FixtureDef fd;
            fd.shape = &shape;
            fd.density = 1.0f;
            bodies[i]->CreateFixture(&fd);
        }
        
//        float G = 0.05;
//        
//        for ( int i = 0; i < numBodies; i++ ) {
//            
//            b2Body* bi = bodies[i];
//            b2Vec2 pi = bi->GetWorldCenter();
//            float mi = bi->GetMass();
//            
//            for ( int k = i; k < numBodies; k++ ) {
//                
//                b2Body* bk = bodies[k];
//                b2Vec2 pk = bk->GetWorldCenter();
//                float mk = bk->GetMass();
//                
//                b2Vec2 delta = pk - pi;
//                float r = delta.Length();
//                float force = G * mi * mk / (r*r);
//                
//                delta.Normalize();
//                bi->ApplyForce(  force * delta, pi );
//                bk->ApplyForce( -force * delta, pk );
//            }
//        }

    }

    void Keyboard(unsigned char key)
    {
        switch (key)
        {
        case 'w':
            {
                b2Vec2 f = m_body->GetWorldVector(b2Vec2(0.0f, -200.0f));
                b2Vec2 p = m_body->GetWorldPoint(b2Vec2(0.0f, 2.0f));
                m_body->ApplyForce(f, p);
            }
            break;

        case 'a':
            {
                m_body->ApplyTorque(50.0f);
            }
            break;

        case 'd':
            {
                m_body->ApplyTorque(-50.0f);
            }
            break;
        }
    }

    static Test* Create()
    {
        return new ApplyForce;
    }

    b2Body* m_body;
    const int numBodies = 5;
    b2Body* bodies[5];
    
};

#endif
