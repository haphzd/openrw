#include <render/MapRenderer.hpp>
#include <render/GameShaders.hpp>
#include <engine/GameWorld.hpp>
#include <engine/GameData.hpp>
#include <engine/GameState.hpp>
#include <ai/PlayerController.hpp>
#include <objects/CharacterObject.hpp>

const char* MapVertexShader = R"(
#version 330
#extension GL_ARB_explicit_attrib_location : enable
#extension GL_ARB_uniform_buffer_object : enable

layout(location = 0) in vec2 position;
out vec2 TexCoord;

uniform mat4 proj;
uniform mat4 view;
uniform mat4 model;

void main()
{
	gl_Position = proj * view * model * vec4(position, 0.0, 1.0);
	// UI space is top to bottom, so flip Y.
	TexCoord = position + vec2( 0.5 );
})";

const char* MapFragmentShader = R"(
#version 330

in vec2 TexCoord;
uniform vec4 colour;
uniform sampler2D spriteTexture;
out vec4 outColour;

void main()
{
	vec4 c = texture(spriteTexture, TexCoord*0.99);
	outColour = vec4(colour.rgb + c.rgb, colour.a * c.a);
})";


MapRenderer::MapRenderer(Renderer* renderer, GameData* _data)
: data(_data), renderer(renderer)
{
	rectGeom.uploadVertices<VertexP2>({
		{-.5f,  .5f},
		{ .5f,  .5f},
		{-.5f, -.5f},
		{ .5f, -.5f}
	});
	rect.addGeometry(&rectGeom);
	rect.setFaceType(GL_TRIANGLE_STRIP);

	std::vector<VertexP2> circleVerts;
	circleVerts.push_back({0.f, 0.f});
	for (int v = 0; v < 181; ++v) {
		circleVerts.push_back({
					0.5f * glm::cos(2*(v/180.f)*glm::pi<float>()),
					0.5f * glm::sin(2*(v/180.f)*glm::pi<float>())
		});
	}
	circleGeom.uploadVertices(circleVerts);
	circle.addGeometry(&circleGeom);
	circle.setFaceType(GL_TRIANGLE_FAN);
	
	rectProg = renderer->createShader(
		MapVertexShader,
		MapFragmentShader
	);
	
	renderer->setUniform(rectProg, "colour", glm::vec4(1.f));
}

#define GAME_MAP_SIZE 4000

void MapRenderer::draw(GameWorld* world, const MapInfo& mi)
{
	renderer->pushDebugGroup("Map");
	renderer->useProgram(rectProg);

	// World out the number of units per tile
	glm::vec2 worldSize(GAME_MAP_SIZE);
	const int mapBlockLine = 8;
	glm::vec2 tileSize = worldSize / (float)mapBlockLine;
	// Determine the scale to show the right number of world units on the screen
	float worldScale = mi.screenSize / mi.worldSize;

	auto proj = renderer->get2DProjection();
	glm::mat4 view, model;
	renderer->setUniform(rectProg, "proj", proj);
	renderer->setUniform(rectProg, "model", glm::mat4());
	renderer->setUniform(rectProg, "colour", glm::vec4(0.f, 0.f, 0.f, 1.f));

	view = glm::translate(view, glm::vec3(mi.screenPosition, 0.f));

	if (mi.clipToSize)
	{
		glBindVertexArray( circle.getVAOName() );
		glBindTexture(GL_TEXTURE_2D, 0);
		glm::mat4 circleView = glm::scale(view, glm::vec3(mi.screenSize));
		renderer->setUniform(rectProg, "view", circleView);
		glEnable(GL_STENCIL_TEST);
		glStencilFunc(GL_ALWAYS, 1, 0xFF);
		glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
		glStencilMask(0xFF);
		glColorMask(0x00, 0x00, 0x00, 0x00);
		glDrawArrays(GL_TRIANGLE_FAN, 0, 182);
		glColorMask(0xFF, 0xFF, 0xFF, 0xFF);
		glStencilFunc(GL_EQUAL, 1, 0xFF);
	}

	view = glm::scale(view, glm::vec3(worldScale));
	view = glm::rotate(view, mi.rotation, glm::vec3(0.f, 0.f, 1.f));
	view = glm::translate(view, glm::vec3(glm::vec2(-1.f, 1.f) * mi.worldCenter, 0.f));
	renderer->setUniform(rectProg, "view", view);

	glBindVertexArray( rect.getVAOName() );
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, 0);

	// radar00 = -x, +y
	// incrementing in X, then Y
	
	int initX = -(mapBlockLine/2);
	int initY = -(mapBlockLine/2);

	for( int m = 0; m < MAP_BLOCK_SIZE; ++m )
	{
		std::string num = (m < 10 ? "0" : "");
		std::string name = "radar" + num +  std::to_string(m);
		auto texture = world->data->textures[{name,""}];
		
		glBindTexture(GL_TEXTURE_2D, texture->getName());
		
		int mX = initX + (m % mapBlockLine);
		int mY = initY + (m / mapBlockLine);
		
		auto tc = glm::vec2(mX, mY) * tileSize + glm::vec2(tileSize/2.f);
		
		glm::mat4 tilemodel = model;
		tilemodel = glm::translate( tilemodel, glm::vec3( tc, 0.f ) );
		tilemodel = glm::scale( tilemodel, glm::vec3( tileSize, 1.f ) );
		
		renderer->setUniform(rectProg, "model", tilemodel);
		
		glDrawArrays( GL_TRIANGLE_STRIP, 0, 4 );
	}

	// From here on out we will work in screenspace
	renderer->setUniform(rectProg, "view", glm::mat4());

	if (mi.clipToSize) {
		glDisable(GL_STENCIL_TEST);
		// We only need the outer ring if we're clipping.
		glBlendFuncSeparate(GL_DST_COLOR, GL_ZERO, GL_ONE, GL_ZERO);
		TextureData::Handle radarDisc = data->findTexture("radardisc");

		glm::mat4 model;
		model = glm::translate(model, glm::vec3(mi.screenPosition, 0.0f));
		model = glm::scale(model, glm::vec3(mi.screenSize*1.07));
		renderer->setUniform(rectProg, "model", model);
		glBindTexture(GL_TEXTURE_2D, radarDisc->getName());
		glDrawArrays( GL_TRIANGLE_STRIP, 0, 4 );
		glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ZERO);
	}
	
	for(auto& blip : world->state->radarBlips)
	{
		glm::vec2 blippos( blip.second.coord );
		if( blip.second.target > 0 )
		{
			GameObject* object = world->getBlipTarget(blip.second);
			if( object )
			{
				blippos = glm::vec2( object->getPosition() );
			}
		}
		
		drawBlip(blippos, view, mi, blip.second.texture);
	}
	
	// Draw the player blip
	auto player = world->pedestrianPool.find(world->state->playerObject);
	if( player )
	{
		glm::vec2 plyblip(player->getPosition());
		float hdg = glm::roll(player->getRotation());
		drawBlip(plyblip, view, mi, "radar_centre", mi.rotation - hdg);
	}

	drawBlip(mi.worldCenter + glm::vec2(0.f, mi.worldSize), view, mi, "radar_north", 0.f, 24.f);

	glBindVertexArray( 0 );
	glBindTexture(GL_TEXTURE_2D, 0);
	glUseProgram(0);
	
	/// @TODO migrate to using the renderer
	renderer->invalidate();
	renderer->popDebugGroup();
}

void MapRenderer::drawBlip(const glm::vec2& coord, const glm::mat4& view, const MapInfo& mi, const std::string& texture, float heading, float size)
{
	glm::vec2 adjustedCoord = coord;
	if (mi.clipToSize)
	{
		float maxDist = mi.worldSize/2.f;
		float centerDist = glm::distance(coord, mi.worldCenter);
		if (centerDist > maxDist) {
			adjustedCoord = mi.worldCenter + ((coord - mi.worldCenter)/centerDist)*maxDist;
		}
	}

	glm::vec3 viewPos(view * glm::vec4(glm::vec2(1.f,-1.f)*adjustedCoord, 0.f, 1.f));
	glm::mat4 model;
	model = glm::translate(model, viewPos);
	model = glm::scale(model, glm::vec3(size));
	model = glm::rotate(model, heading, glm::vec3(0.f, 0.f, 1.f));
	renderer->setUniform(rectProg, "model", model);

	GLuint tex = 0;
	if ( !texture.empty() )
	{
		auto sprite= data->findTexture(texture);
		tex = sprite->getName();
		renderer->setUniform(rectProg, "colour", glm::vec4(0.f, 0.f, 0.f, 1.f));
	}
	else
	{
		renderer->setUniform(rectProg, "colour", glm::vec4(1.0f, 1.0f, 1.0f, 1.f));
	}
	
	glBindTexture(GL_TEXTURE_2D, tex);

	glDrawArrays( GL_TRIANGLE_STRIP, 0, 4 );
}
