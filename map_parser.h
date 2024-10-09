/*
* Quake MAP parser.
*
* Grammar:
* ----------------------------------------------------------------
* Terminals = {key, value, plane, texture, offset, scale, rotation}
* Non Terminals = {map, entity, property, brush, face}

* map         -> entity *entity
* entity      -> *property *brush
* property    -> key value
* brush       -> *face
* face        -> plane texture-name xOffset yOffset rotation xScale yScale
*
* A plane is defined by 3 vertices.
* A property is always a pair of strings.
*
* For more information, see: https://quakewiki.org/wiki/Quake_Map_Format
*/

#ifndef _PARSER_H_
#define _PARSER_H_

#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <vector>

enum MapVersion
{
	QUAKE,
	VALVE_220
};

enum TokenType
{
	LBRACE,
	RBRACE,
	LPAREN,
	RPAREN,
	LBRACKET,
	RBRACKET,
	NUMBER,
	STRING,
	COMMENT,
	TEXNAME,
	UNKNOWN,
	END_OF_INPUT
};

struct MapVertex
{
	double x, y, z;
};

struct Face
{
	MapVertex		vertices[3];     // Define the plane.
	std::string		textureName;
	double			xOffset, yOffset;
	double			rotation;
	double			xScale, yScale;

	// Valve 220 texture format
	double			tx1, ty1, tz1, tOffset1;
	double			tx2, ty2, tz2, tOffset2;
};

struct Brush
{
	std::vector<Face> faces;
};

struct Property
{
	std::string key;
	std::string value;
};

struct Entity
{
	std::vector<Property> properties;
	std::vector<Brush>    brushes;
};

struct Map
{
	std::vector<Entity> entities;
};

Map getMap(char* mapData, size_t mapDataLength, MapVersion mapVersion = QUAKE);



/* 
* 
* IMPLEMENTATION 
* 
*/



#if defined(MAP_PARSER_IMPLEMENTATION)

static int			g_InputLength;
static int			g_LineNo;
static MapVersion	g_MapVersion;

static int advanceCursor(int* pos, int steps)
{
	int advanceable = g_InputLength - *pos + steps;
	if (advanceable > 0) {
		*pos += advanceable;
		return advanceable;
	}
	return 0;
}

static void advanceToNextNonWhitespace(char* c, int* pos)
{
	char* cur = c + *pos;
	while (isspace(*cur)) {
		if (*cur == '\n') { // TODO: Get Env-Newline?
			g_LineNo++;
		}
		cur++; *pos += 1;
	}
}

static void advanceToNextWhitespaceOrLinebreak(char** c, int* pos)
{
	while (!isspace(**c) && **c != '\n' && **c != '\r') {
		*c += 1; *pos += 1;
	}
}

static void skipLinebreaks(char* c, int* pos)
{
	char* cur = c + *pos;
	while (*cur == '\r' || *cur == '\n') {
		cur++; *pos += 1; g_LineNo++;
	}
}

static void advanceToNextLine(char* c, int* pos)
{
	char* cur = c + *pos;
	while (*cur != '\r' && *cur != '\n') {
		cur++; *pos += 1;
	}
	if (*cur == '\r') {
		cur++; *pos += 1;
	}
	if (*cur == '\n') {
		cur++; *pos += 1;
		g_LineNo++;
	}
}

static std::string getString(char* c, int* pos)
{
	char* cur = c + *pos;

	cur++; *pos += 1; // advance over "
	std::string result = "";
	while (*cur != '\"') {
		result += *cur; *pos += 1; cur++;
	}
	*pos += 1; // advance over "

	return result;
}

static TokenType getToken(char* c, int* pos)
{
	if (*pos >= g_InputLength) {
		return END_OF_INPUT;
	}

	TokenType result = UNKNOWN;

	advanceToNextNonWhitespace(c, pos);
	skipLinebreaks(c, pos);

	while (*(c + *pos) == '/') { // Skip over all comments
		advanceToNextLine(c, pos);
		advanceToNextNonWhitespace(c, pos);
	}

	char* cur = c + *pos;

	if (*cur == '{') {
		result = LBRACE;
	}
	else if (*cur == '}') {
		result = RBRACE;
	}
	else if (*cur == '(') {
		result = LPAREN;
	}
	else if (*cur == ')') {
		result = RPAREN;
	}
	else if (*cur == '[') {
		result = LBRACKET;
	}
	else if (*cur == ']') {
		result = RBRACKET;
	}
	else if (*cur == '"') {
		result = STRING;
	}
	else if (*cur >= '0' && *cur <= '9' || *cur == '-') { // TODO: MINUS own token and check again if really NUMBER
		result = NUMBER;
	}
	else if (*cur >= 0x21 && *cur <= 0x7E) { // '!' - '~'
		result = TEXNAME;
	}
	else {
		result = UNKNOWN;
		*pos += 1;
	}

	return result;
}

static std::string tokenToString(TokenType tokenType)
{
	switch (tokenType)
	{
	case LBRACE: return "LBRACE"; break;
	case RBRACE: return "RBRACE"; break;
	case LPAREN: return "LPAREN"; break;
	case RPAREN: return "RPAREN"; break;
	case NUMBER: return "NUMBER"; break;
	case STRING: return "STRING"; break;
	case COMMENT: return "COMMENT"; break;
	case TEXNAME: return "TEXNAME"; break;
	case UNKNOWN: return "UNKNOWN"; break;
	default: return "!!! H E L P !!!";
	}
}

static bool check(TokenType got, TokenType expected)
{
	if (expected != got) {
		fprintf(stderr, "ERROR: Expected Token: %s, but got: %s in line %d\n",
			tokenToString(expected).c_str(), tokenToString(got).c_str(), g_LineNo);
		exit(-1);
	}

	return true;
}

static double parseNumber(char* c, int* pos)
{
	char* cur = c + *pos;
	std::string number = "";
	while (*cur == '-' || *cur >= '0' && *cur <= '9' || *cur == '.') {
		number += *cur; cur++; *pos += 1;
	}
	return atof(number.c_str());
}

static MapVertex getVertex(char* c, int* pos)
{
	MapVertex v = { };
	std::vector<double> values;
	for (int i = 0; i < 3; i++) { // A face is defined by 3 vertices.
		advanceToNextNonWhitespace(c, pos);
		double value = parseNumber(c, pos);
		values.push_back(value);
	}

	v.x = values[0];
	v.y = values[1];
	v.z = values[2];

	return v;
}

static double getNumber(char* c, int* pos)
{
	char* cur = c + *pos;
	std::string number = "";
	while (*cur == '-' || *cur >= '0' && *cur <= '9' || *cur == '.' || *cur == 'e') {
		number += *cur; cur++; *pos += 1;
	}
	return atof(number.c_str());
}

static std::string getTextureName(char* c, int* pos)
{
	char* cur = c + *pos;

	std::string textureName = "";
	char* end = cur;
	advanceToNextWhitespaceOrLinebreak(&end, pos);
	while (cur != end) {
		textureName += *cur; cur++;
	}

	return textureName;
}

static Property getProperty(char* c, int* pos)
{
	check(getToken(c, pos), STRING);
	std::string key = getString(c, pos);
	check(getToken(c, pos), STRING);
	std::string value = getString(c, pos);

	return { key, value };
}

/*
* TODO: getFace and getFaceValve220 are fairly similar. Try to compress this?
*/
static Face getFace(char* c, int* pos)
{
	Face face = { };

	/* 3 Vertices defining the plane */
	for (size_t i = 0; i < 3; ++i) {
		check(getToken(c, pos), LPAREN); *pos += 1;
		check(getToken(c, pos), NUMBER);
		double x = getNumber(c, pos);
		check(getToken(c, pos), NUMBER);
		double y = getNumber(c, pos);
		check(getToken(c, pos), NUMBER);
		double z = getNumber(c, pos);
		check(getToken(c, pos), RPAREN); *pos += 1;
		face.vertices[i] = { x, y, z };
	}

	/* Texture stuff */
	check(getToken(c, pos), TEXNAME);
	face.textureName = getTextureName(c, pos);
	check(getToken(c, pos), NUMBER);
	face.xOffset = getNumber(c, pos);
	check(getToken(c, pos), NUMBER);
	face.yOffset = getNumber(c, pos);
	check(getToken(c, pos), NUMBER);
	face.rotation = getNumber(c, pos);
	check(getToken(c, pos), NUMBER);
	face.xScale = getNumber(c, pos);
	check(getToken(c, pos), NUMBER);
	face.yScale = getNumber(c, pos);

	/*
	* Some map formats do have extra texture information.
	* There seems to be the Quake2 map format that has 3 additional
	* numbers. I could not find the specification for those.
	* So for now, if this is the case (Q2-format) then consume those
	* numbers and throw them away.
	* 
	* TODO: Figure out what those 3 extra numbers are in Q2.
	*/
	int *currentPos = pos; // save original (eg. do a lookahead)
	if (getToken(c, currentPos) == NUMBER) { 

		getToken(c, pos); // if the lookahead returned a NUMBER get the token again to advance pos.
		getNumber(c, pos);
		check(getToken(c, pos), NUMBER);
		getNumber(c, pos);
		check(getToken(c, pos), NUMBER);
		getNumber(c, pos);
	}

	return face;
}

static Face getFaceValve220(char* c, int* pos)
{
	Face face = { };

	/* 3 Vertices defining the plane */
	for (size_t i = 0; i < 3; ++i) {
		check(getToken(c, pos), LPAREN); *pos += 1;
		check(getToken(c, pos), NUMBER);
		double x = getNumber(c, pos);
		check(getToken(c, pos), NUMBER);
		double y = getNumber(c, pos);
		check(getToken(c, pos), NUMBER);
		double z = getNumber(c, pos);
		check(getToken(c, pos), RPAREN); *pos += 1;
		face.vertices[i] = { x, y, z };
	}

	check(getToken(c, pos), TEXNAME);
	face.textureName = getTextureName(c, pos);

	/* Texture stuff */
	check(getToken(c, pos), LBRACKET); *pos += 1;
	check(getToken(c, pos), NUMBER);
	face.tx1 = getNumber(c, pos);
	check(getToken(c, pos), NUMBER);
	face.ty1 = getNumber(c, pos);
	check(getToken(c, pos), NUMBER);
	face.tz1 = getNumber(c, pos);
	check(getToken(c, pos), NUMBER);
	face.tOffset1 = getNumber(c, pos);
	check(getToken(c, pos), RBRACKET); *pos += 1;

	check(getToken(c, pos), LBRACKET); *pos += 1;
	check(getToken(c, pos), NUMBER);
	face.tx2 = getNumber(c, pos);
	check(getToken(c, pos), NUMBER);
	face.ty2 = getNumber(c, pos);
	check(getToken(c, pos), NUMBER);
	face.tz2 = getNumber(c, pos);
	check(getToken(c, pos), NUMBER);
	face.tOffset2 = getNumber(c, pos);
	check(getToken(c, pos), RBRACKET); *pos += 1;

	check(getToken(c, pos), NUMBER);
	face.rotation = getNumber(c, pos);
	check(getToken(c, pos), NUMBER);
	face.xScale = getNumber(c, pos);
	check(getToken(c, pos), NUMBER);
	face.yScale = getNumber(c, pos);

	return face;
}

static Brush getBrush(char* c, int* pos)
{
	Brush brush = { };

	while (getToken(c, pos) == LPAREN) {
		if (g_MapVersion == VALVE_220) {
			brush.faces.push_back(getFaceValve220(c, pos));
		}
		else {
			brush.faces.push_back(getFace(c, pos));
		}
	}

	int faceCount = brush.faces.size();
	if (faceCount < 6) {
		fprintf(stderr, "WARNING (Line %d): Brush found with only %d faces!\n", g_LineNo, faceCount);
	}

	return brush;
}

/**
* I assume that the grammar does not allow a brush *before* a property within an entity!
*/
static Entity getEntity(char* c, int* pos)
{
	Entity e = { };

	while (getToken(c, pos) == STRING) {
		e.properties.push_back(getProperty(c, pos));
	}

	while (getToken(c, pos) == LBRACE) {
		*pos += 1;
		e.brushes.push_back(getBrush(c, pos));
		check(getToken(c, pos), RBRACE);
		*pos += 1;
	}

	check(getToken(c, pos), RBRACE); *pos += 1;

	return e;
}

Map getMap(char* mapData, size_t mapDataLength, MapVersion mapVersion)
{
	Map map = {};

	g_InputLength = mapDataLength;
	g_LineNo = 1; // Editors often start at line 1
	g_MapVersion = mapVersion;
	int pos = 0;
	check(getToken(&mapData[0], &pos), LBRACE); // Map file must start with an entity!
	while (getToken(&mapData[0], &pos) == LBRACE) {
		pos++;
		map.entities.push_back(getEntity(&mapData[0], &pos));
	}

	return map;
}

#endif

#endif
