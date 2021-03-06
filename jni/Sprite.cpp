#include "Sprite.hpp"
#include <OgreTextureManager.h>

/*
    OgreSprites

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public
    License along with this library; if not, write to the Free
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

    Written by Andrew C Lytle, June 2007.

    Developed based on code by H. HernÃ¡n Moraldo from Moraldo Games
    www.hernan.moraldo.com.ar/pmenglish/field.php
*/

#include <OgreMesh.h>
#include <OgreHardwareBuffer.h>
#include "Sprite.hpp"

#define OGRE2D_MINIMAL_HARDWARE_BUFFER_SIZE 120

namespace rcd
{
    const Sprite::Rect Sprite::FULL_SPRITE(-1000.0f, -1000.0f, -1000.0f, -1000.0f);
    const Sprite::Rect Sprite::FULL_SCREEN(-1000.0f, -1000.0f, -1000.0f, -1000.0f);

    //-----------------------------------------------------------------------
    Sprite::Sprite() :
        _metricScreen(OSPRITE_METRIC_RELATIVE),
        _metricSprite(OSPRITE_METRIC_PIXELS)
    {
    }
    //-----------------------------------------------------------------------
    Sprite::~Sprite()
    {
    }
    //-----------------------------------------------------------------------
    void Sprite::renderQueueStarted(Ogre::uint8 queueGroupId, const Ogre::String &invocation, bool &skipThisInvocation)
    {
       if (!afterQueue && queueGroupId==targetQueue)
          renderBuffer();
    }
    //-----------------------------------------------------------------------
    void Sprite::renderQueueEnded(Ogre::uint8 queueGroupId, const Ogre::String &invocation, bool &repeatThisInvocation)
    {
       if (afterQueue && queueGroupId==targetQueue)
          renderBuffer();
    }
    //-----------------------------------------------------------------------
    void Sprite::Init(Ogre::SceneManager* sceneMan, Ogre::Viewport* viewPort, Ogre::uint8 targetQueue, bool afterQueue)
    {
        // Save scene manager data
        Sprite::sceneMan = sceneMan;
        Sprite::afterQueue = afterQueue;
        Sprite::targetQueue = targetQueue;

        // Ensure our hardware buffer is set to zero
        hardwareBuffer.setNull();

        // Set this object as a render queue listener with Ogre
        sceneMan->addRenderQueueListener(this);

        // Gather viewport info
        _vpWidth = viewPort->getActualWidth();
        _vpHeight = viewPort->getActualHeight();
        _vpHalfWidth = (int)(_vpWidth / 2);
        _vpHalfHeight = (int)(_vpHeight / 2);
    }
    //-----------------------------------------------------------------------
    void Sprite::Shutdown()
    {
        // Destroy the hardware buffer
        if (!hardwareBuffer.isNull())
            destroyHardwareBuffer();

        // Delist our renderqueuelistener
        sceneMan->removeRenderQueueListener(this);
    }
    //-----------------------------------------------------------------------
    void Sprite::SetScreenMetric(OSMetrics metric)
    {
        _metricScreen = metric;
    }
    //-----------------------------------------------------------------------
    void Sprite::SetSpriteMetric(OSMetrics metric)
    {
        if(metric == OSPRITE_METRIC_RELATIVE || metric == OSPRITE_METRIC_PIXELS)
            _metricSprite = metric;
        else
            _metricSprite = OSPRITE_METRIC_RELATIVE;
    }
    //-----------------------------------------------------------------------
    void Sprite::SetSpriteLocation(const std::string& pathName)
    {
        spriteLocation = pathName;

        Ogre::ResourceGroupManager::getSingleton().addResourceLocation(pathName, "APKFileSystem", "OgreSprites");
        Ogre::ResourceGroupManager::getSingleton().initialiseResourceGroup("OgreSprites");
    }
    //-----------------------------------------------------------------------
    void Sprite::LoadSprite(const std::string& spriteName)
    {
        Ogre::TextureManager::getSingleton().load(spriteName, "OgreSprites");
    }
    //-----------------------------------------------------------------------
    void Sprite::DrawSprite(const std::string& spriteName, float x, float y, const Ogre::ColourValue &color, float alpha, bool additiveBlend)
    {
        // Retrieve pointer to texture resource
        Ogre::TexturePtr texturePtr = Ogre::TextureManager::getSingleton().getByName(spriteName);

        // This is the size of the original image data (pixels)
        int iSpriteWidth = (int)texturePtr->getWidth();
        int iSpriteHeight = (int)texturePtr->getHeight();

        // Get texture handle from texture resource
        SpriteElement spriteElement;
        spriteElement.texHandle = texturePtr->getHandle();

        // Convert destination start to Pixels
        float fPixelStartX = 0;
        float fPixelStartY = 0;
        convertScreenMetrics(_metricScreen, x, y, OSPRITE_METRIC_PIXELS, fPixelStartX, fPixelStartY);
        int iPixelEndX = (int)fPixelStartX + (int)iSpriteWidth;
        int iPixelEndY = (int)fPixelStartY + (int)iSpriteHeight;

        // Convert from pixels to Ogre
        convertScreenMetrics(OSPRITE_METRIC_PIXELS, fPixelStartX, fPixelStartY, OSPRITE_METRIC_OGRE, spriteElement.x1, spriteElement.y1);
        convertScreenMetrics(OSPRITE_METRIC_PIXELS, iPixelEndX, iPixelEndY, OSPRITE_METRIC_OGRE, spriteElement.x2, spriteElement.y2);

        // We want to draw the entire sprite
        spriteElement.tx1 = 0.0f;
        spriteElement.ty1 = 0.0f;
        spriteElement.tx2 = 1.0f;
        spriteElement.ty2 = 1.0f;

        // save alpha value
        spriteElement.alpha = alpha;
		spriteElement.color = color;
		spriteElement.additiveBlend = additiveBlend;

        // Add this sprite to our render list
        sprites.push_back(spriteElement);
    }
    //-----------------------------------------------------------------------
    void Sprite::DrawSprite(const std::string& spriteName, float x, float y, const Sprite::Rect& spriteRect, const Ogre::ColourValue &color, float alpha, bool additiveBlend)
    {
        // Retrieve pointer to texture resource
        Ogre::TexturePtr texturePtr = Ogre::TextureManager::getSingleton().getByName(spriteName);

        // This is the size of the original image data (pixels)
        int iSpriteWidth = (int)texturePtr->getWidth();
        int iSpriteHeight = (int)texturePtr->getHeight();

        // Get texture handle from texture resource
        SpriteElement spriteElement;
        spriteElement.texHandle = texturePtr->getHandle();

        // Drawing size
        int iDrawingWidth = (spriteRect.x2 - spriteRect.x1);
        int iDrawingHeight = (spriteRect.y2 - spriteRect.y1);

        // Convert destination start to Pixels
        float fPixelStartX = 0;
        float fPixelStartY = 0;
        convertScreenMetrics(_metricScreen, x, y, OSPRITE_METRIC_PIXELS, fPixelStartX, fPixelStartY);
        int iPixelEndX = (int)fPixelStartX + (int)iDrawingWidth;
        int iPixelEndY = (int)fPixelStartY + (int)iDrawingHeight;

        // Convert from pixels to Ogre
        convertScreenMetrics(OSPRITE_METRIC_PIXELS, fPixelStartX, fPixelStartY, OSPRITE_METRIC_OGRE, spriteElement.x1, spriteElement.y1);
        convertScreenMetrics(OSPRITE_METRIC_PIXELS, iPixelEndX, iPixelEndY, OSPRITE_METRIC_OGRE, spriteElement.x2, spriteElement.y2);

        // We want to draw only a portion of the sprite
        spriteElement.tx1 = spriteElement.ty1 = 0.0f;
        spriteElement.tx2 = spriteElement.ty2 = 1.0f;

        if(spriteRect != FULL_SPRITE) {
            if(_metricSprite == OSPRITE_METRIC_RELATIVE) {
                spriteElement.tx1 = spriteRect.x1;
                spriteElement.ty1 = spriteRect.y1;
                spriteElement.tx2 = spriteRect.x2;
                spriteElement.ty2 = spriteRect.y2;
            }
            else if(_metricSprite == OSPRITE_METRIC_PIXELS) {

                spriteElement.tx1 = (float)(spriteRect.x1 / iSpriteWidth);
                spriteElement.ty1 = (float)(spriteRect.y1 / iSpriteHeight);
                spriteElement.tx2 = (float)(spriteRect.x2 / iSpriteWidth);
                spriteElement.ty2 = (float)(spriteRect.y2 / iSpriteHeight);
            }
        }

        // save alpha value
        spriteElement.alpha = alpha;
		spriteElement.color = color;
		spriteElement.additiveBlend = additiveBlend;

        // Add this sprite to our render list
        sprites.push_back(spriteElement);
    }
    //-----------------------------------------------------------------------
	void Sprite::DrawSprite(const std::string& spriteName, const Sprite::Rect& destRect, const Sprite::Rect& spriteRect, const Ogre::ColourValue &color, float alpha, bool additiveBlend)
    {
        // Retrieve pointer to texture resource
        Ogre::TexturePtr texturePtr = Ogre::TextureManager::getSingleton().getByName(spriteName);

        // Get texture handle from texture resource
        SpriteElement spriteElement;
        spriteElement.texHandle = texturePtr->getHandle();

        // This is the size of the original image data (pixels)
        int iSpriteWidth = (int)texturePtr->getWidth();
        int iSpriteHeight = (int)texturePtr->getHeight();

        if(destRect != FULL_SCREEN) {
            // Convert destination start to Pixels
            float fPixelStartX = 0;
            float fPixelStartY = 0;
            convertScreenMetrics(_metricScreen, destRect.x1, destRect.y1, OSPRITE_METRIC_PIXELS, fPixelStartX, fPixelStartY);

            // Convert size to pixels
            float fPixelEndX = 0;
            float fPixelEndY = 0;
            convertScreenMetrics(_metricScreen, destRect.x2, destRect.y2, OSPRITE_METRIC_PIXELS, fPixelEndX, fPixelEndY);

            // Convert from pixels to Ogre
            convertScreenMetrics(OSPRITE_METRIC_PIXELS, fPixelStartX, fPixelStartY, OSPRITE_METRIC_OGRE, spriteElement.x1, spriteElement.y1);
            convertScreenMetrics(OSPRITE_METRIC_PIXELS, fPixelEndX, fPixelEndY, OSPRITE_METRIC_OGRE, spriteElement.x2, spriteElement.y2);
        }
        else {
            spriteElement.x1 = -1;
            spriteElement.x2 = 1;
            spriteElement.y1 = 1;
            spriteElement.y2 = -1;
        }

        // We want to draw only a portion of the sprite
        spriteElement.tx1 = spriteElement.ty1 = 0.0f;
        spriteElement.tx2 = spriteElement.ty2 = 1.0f;

        if(spriteRect != FULL_SPRITE) {
            if(_metricSprite == OSPRITE_METRIC_RELATIVE) {
                spriteElement.tx1 = spriteRect.x1;
                spriteElement.ty1 = spriteRect.y1;
                spriteElement.tx2 = spriteRect.x2;
                spriteElement.ty2 = spriteRect.y2;
            }
            else if(_metricSprite == OSPRITE_METRIC_PIXELS) {
                spriteElement.tx1 = (float)(spriteRect.x1 / iSpriteWidth);
                spriteElement.ty1 = (float)(spriteRect.y1 / iSpriteHeight);
                spriteElement.tx2 = (float)(spriteRect.x2 / iSpriteWidth);
                spriteElement.ty2 = (float)(spriteRect.y2 / iSpriteHeight);
            }
        }

        // save alpha value
        spriteElement.alpha = alpha;

		spriteElement.color = color;
		spriteElement.additiveBlend = additiveBlend;

        // Add this sprite to our render list
        sprites.push_back(spriteElement);
    }
    //-----------------------------------------------------------------------
    void Sprite::renderBuffer()
    {
       Ogre::RenderSystem* rs=Ogre::Root::getSingleton().getRenderSystem();
       std::list<SpriteElement>::iterator currSpr, endSpr;

       VertexChunk thisChunk;
       std::list<VertexChunk> chunks;

       unsigned int newSize;

       newSize = (int)(sprites.size())*6;
       if (newSize<OGRE2D_MINIMAL_HARDWARE_BUFFER_SIZE)
          newSize=OGRE2D_MINIMAL_HARDWARE_BUFFER_SIZE;

       // grow hardware buffer if needed
       if (hardwareBuffer.isNull() || hardwareBuffer->getNumVertices()<newSize)
       {
          if (!hardwareBuffer.isNull())
             destroyHardwareBuffer();

          createHardwareBuffer(newSize);
       }

       // If we have no sprites this frame, bail here
       if (sprites.empty()) return;

       // write quads to the hardware buffer, and remember chunks
       float* buffer;
       float z=-1;

       buffer=(float*)hardwareBuffer->lock(Ogre::HardwareBuffer::HBL_DISCARD);

       endSpr=sprites.end();
       currSpr=sprites.begin();
       thisChunk.texHandle=currSpr->texHandle;
       thisChunk.vertexCount=0;
       while (currSpr!=endSpr)
       {
           thisChunk.alpha = currSpr->alpha;
		   thisChunk.color = currSpr->color;
		   thisChunk.additiveBlend = currSpr->additiveBlend;

          // 1st point (left bottom)
          *buffer=currSpr->x1; buffer++;
          *buffer=currSpr->y2; buffer++;
          *buffer=z; buffer++;
          *buffer=currSpr->tx1; buffer++;
          *buffer=currSpr->ty2; buffer++;
          // 2st point (right top)
          *buffer=currSpr->x2; buffer++;
          *buffer=currSpr->y1; buffer++;
          *buffer=z; buffer++;
          *buffer=currSpr->tx2; buffer++;
          *buffer=currSpr->ty1; buffer++;
          // 3rd point (left top)
          *buffer=currSpr->x1; buffer++;
          *buffer=currSpr->y1; buffer++;
          *buffer=z; buffer++;
          *buffer=currSpr->tx1; buffer++;
          *buffer=currSpr->ty1; buffer++;

          // 4th point (left bottom)
          *buffer=currSpr->x1; buffer++;
          *buffer=currSpr->y2; buffer++;
          *buffer=z; buffer++;
          *buffer=currSpr->tx1; buffer++;
          *buffer=currSpr->ty2; buffer++;
          // 5th point (right bottom)
          *buffer=currSpr->x2; buffer++;
          *buffer=currSpr->y1; buffer++;
          *buffer=z; buffer++;
          *buffer=currSpr->tx2; buffer++;
          *buffer=currSpr->ty1; buffer++;
          // 6th point (right top)
          *buffer=currSpr->x2; buffer++;
          *buffer=currSpr->y2; buffer++;
          *buffer=z; buffer++;
          *buffer=currSpr->tx2; buffer++;
          *buffer=currSpr->ty2; buffer++;

          // remember this chunk
          thisChunk.vertexCount+=6;
          currSpr++;
          if (currSpr==endSpr || thisChunk.texHandle!=currSpr->texHandle || thisChunk.alpha != currSpr->alpha ||
			  thisChunk.color != currSpr->color || thisChunk.additiveBlend != currSpr->additiveBlend)
          {
             chunks.push_back(thisChunk);
             if (currSpr!=endSpr)
             {
                thisChunk.texHandle=currSpr->texHandle;
                thisChunk.vertexCount=0;
             }
          }
       }

       hardwareBuffer->unlock();

       // set up...
       prepareForRender();

       // do the real render!
       Ogre::TexturePtr tp;
       std::list<VertexChunk>::iterator currChunk, endChunk;

       endChunk=chunks.end();
       renderOp.vertexData->vertexStart=0;
       for (currChunk=chunks.begin(); currChunk!=endChunk; currChunk++)
       {
          renderOp.vertexData->vertexCount=currChunk->vertexCount;
          tp=Ogre::TextureManager::getSingleton().getByHandle(currChunk->texHandle);
          rs->_setTexture(0, true, tp->getName());

		  if (currChunk->additiveBlend)
			rs->_setSceneBlending(Ogre::SBF_SOURCE_ALPHA, Ogre::SBF_ONE, Ogre::SBO_ADD);
		  else
			rs->_setSceneBlending(Ogre::SBF_SOURCE_ALPHA, Ogre::SBF_ONE_MINUS_SOURCE_ALPHA, Ogre::SBO_ADD);

		  Ogre::LayerBlendModeEx colorBlendMode;
		  if (currChunk->color != Ogre::ColourValue::White)
		  {
			  colorBlendMode.blendType = Ogre::LBT_COLOUR;
			  colorBlendMode.source1 = Ogre::LBS_TEXTURE;
			  colorBlendMode.source2 = Ogre::LBS_MANUAL;
			  colorBlendMode.operation = Ogre::LBX_MODULATE;
			  colorBlendMode.colourArg2 = currChunk->color;
		  }
		  else
		  {
			  colorBlendMode.blendType=Ogre::LBT_COLOUR;
			  colorBlendMode.source1=Ogre::LBS_TEXTURE;
			  colorBlendMode.source2=Ogre::LBS_CURRENT;
			  colorBlendMode.operation=Ogre::LBX_SOURCE1;
			  colorBlendMode.alphaArg1 = 1.0f;
			  colorBlendMode.alphaArg2 = 0.0f;
		  }
		  rs->_setTextureBlendMode(0, colorBlendMode);

          Ogre::LayerBlendModeEx alphaBlendMode;
          alphaBlendMode.blendType=Ogre::LBT_ALPHA;
          alphaBlendMode.source1=Ogre::LBS_TEXTURE;
          alphaBlendMode.source2=Ogre::LBS_MANUAL;
          alphaBlendMode.operation=Ogre::LBX_BLEND_MANUAL;
          alphaBlendMode.factor = currChunk->alpha;
          rs->_setTextureBlendMode(0, alphaBlendMode);

          rs->_render(renderOp);

          renderOp.vertexData->vertexStart+=currChunk->vertexCount;
       }

       // sprites go home!
       sprites.clear();
    }
    //-----------------------------------------------------------------------
    void Sprite::prepareForRender()
    {
       Ogre::LayerBlendModeEx colorBlendMode;
       Ogre::LayerBlendModeEx alphaBlendMode;
       Ogre::TextureUnitState::UVWAddressingMode uvwAddressMode;

       Ogre::RenderSystem* rs=Ogre::Root::getSingleton().getRenderSystem();

       colorBlendMode.blendType=Ogre::LBT_COLOUR;
       colorBlendMode.source1=Ogre::LBS_TEXTURE;
       colorBlendMode.source2=Ogre::LBS_CURRENT;
       colorBlendMode.operation=Ogre::LBX_SOURCE1;
	   colorBlendMode.alphaArg1 = 1.0f;
	   colorBlendMode.alphaArg2 = 0.0f;

       alphaBlendMode.blendType=Ogre::LBT_ALPHA;
       alphaBlendMode.source1=Ogre::LBS_TEXTURE;
       alphaBlendMode.source2=Ogre::LBS_MANUAL;
       alphaBlendMode.operation=Ogre::LBX_BLEND_MANUAL;
	   alphaBlendMode.alphaArg1 = 1.0f;
	   alphaBlendMode.alphaArg2 = 0.0f;

       uvwAddressMode.u=Ogre::TextureUnitState::TAM_CLAMP;
       uvwAddressMode.v=Ogre::TextureUnitState::TAM_CLAMP;
       uvwAddressMode.w=Ogre::TextureUnitState::TAM_CLAMP;

       rs->_setWorldMatrix(Ogre::Matrix4::IDENTITY);
       rs->_setViewMatrix(Ogre::Matrix4::IDENTITY);
       rs->_setProjectionMatrix(Ogre::Matrix4::IDENTITY);
       rs->_setTextureMatrix(0, Ogre::Matrix4::IDENTITY);
       rs->_setTextureCoordSet(0, 0);
       rs->_setTextureCoordCalculation(0, Ogre::TEXCALC_NONE);
       rs->_setTextureUnitFiltering(0, Ogre::FO_LINEAR, Ogre::FO_LINEAR, Ogre::FO_POINT);
       rs->_setTextureBlendMode(0, colorBlendMode);
       rs->_setTextureBlendMode(0, alphaBlendMode);
       rs->_setTextureAddressingMode(0, uvwAddressMode);
       rs->_disableTextureUnitsFrom(1);
       rs->setLightingEnabled(false);
       rs->_setFog(Ogre::FOG_NONE);
       rs->_setCullingMode(Ogre::CULL_NONE);
       rs->_setDepthBufferParams(false, false);
       rs->_setColourBufferWriteEnabled(true, true, true, false);
       rs->setShadingType(Ogre::SO_GOURAUD);
       rs->_setPolygonMode(Ogre::PM_SOLID);
       rs->unbindGpuProgram(Ogre::GPT_FRAGMENT_PROGRAM);
       rs->unbindGpuProgram(Ogre::GPT_VERTEX_PROGRAM);
       rs->_setSceneBlending(Ogre::SBF_SOURCE_ALPHA, Ogre::SBF_ONE_MINUS_SOURCE_ALPHA);
       rs->_setAlphaRejectSettings(Ogre::CMPF_ALWAYS_PASS, 0, false);
    }
    //-----------------------------------------------------------------------
    void Sprite::createHardwareBuffer(unsigned int size)
    {
       Ogre::VertexDeclaration* vd;

       renderOp.vertexData=new Ogre::VertexData;
       renderOp.vertexData->vertexStart=0;

       vd=renderOp.vertexData->vertexDeclaration;
       vd->addElement(0, 0, Ogre::VET_FLOAT3, Ogre::VES_POSITION);
       vd->addElement(0, Ogre::VertexElement::getTypeSize(Ogre::VET_FLOAT3),
          Ogre::VET_FLOAT2, Ogre::VES_TEXTURE_COORDINATES);

       hardwareBuffer=Ogre::HardwareBufferManager::getSingleton().createVertexBuffer(
          vd->getVertexSize(0),
          size,// buffer size
          Ogre::HardwareBuffer::HBU_DYNAMIC_WRITE_ONLY_DISCARDABLE,
          false);// use shadow buffer? no

       renderOp.vertexData->vertexBufferBinding->setBinding(0, hardwareBuffer);

       renderOp.operationType=Ogre::RenderOperation::OT_TRIANGLE_LIST;
       renderOp.useIndexes=false;

    }
    //-----------------------------------------------------------------------
    void Sprite::destroyHardwareBuffer()
    {
       delete renderOp.vertexData;
       renderOp.vertexData=0;
       hardwareBuffer.setNull();
    }
    //-----------------------------------------------------------------------
    void Sprite::convertScreenMetrics(OSMetrics metricFrom, const float sx, const float sy, OSMetrics metricTo, float& dx, float& dy)
    {
        // trivial case
        if(metricFrom == metricTo) {
            dx = sx;
            dy = sy;
            return;
        }

        // Convert from pixels ..
        if(metricFrom == OSPRITE_METRIC_PIXELS) {
            // .. to Ogre.
            if(metricTo == OSPRITE_METRIC_OGRE) {
                dx = (sx / _vpHalfWidth) - 1;
                dy = 1 - (sy / _vpHalfHeight);
            }
            // .. to relative.
            else if(metricTo == OSPRITE_METRIC_RELATIVE) {
                dx = (sx / (float)_vpWidth);
                dy = (sy / (float)_vpHeight);
            }
        }
        // Convert from relative ..
        else if(metricFrom == OSPRITE_METRIC_RELATIVE) {
            // .. to Ogre.
            if(metricTo == OSPRITE_METRIC_OGRE) {
                dx = (sx * 2) - 1;
                dy = (sy * -2) + 1;
                return;
            }
            // .. to pixels.
            else if(metricTo == OSPRITE_METRIC_PIXELS)  {
                dx = (sx * _vpWidth);
                dy = (sy * _vpHeight);
                return;
            }
        }
        // Convert from ogre ..
        else if(metricFrom == OSPRITE_METRIC_OGRE) {
            // .. to pixels.
            if(metricTo == OSPRITE_METRIC_PIXELS) {
                float relx = (sx + 1) / 2;
                float rely = (sy - 1) / (-2);

                dx = (relx * _vpWidth);
                dy = (rely * _vpHeight);
                return;
            }
            // .. to relative.
            else if(metricTo == OSPRITE_METRIC_RELATIVE) {
                dx = (sx + 1) / 2;
                dy = (sy - 1) / (-2);
                return;
            }
        }
    }
}
