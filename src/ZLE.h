//     _____               _     _      _       _                   _   _____    _ _ _
//    |__  /___  _ __ ___ | |__ (_) ___( )___  | |    _____   _____| | | ____|__| (_) |_ ___  _ __
//      / // _ \| '_ ` _ \| '_ \| |/ _ \// __| | |   / _ \ \ / / _ \ | |  _| / _` | | __/ _ \| '__|
//     / /| (_) | | | | | | |_) | |  __/ \__ \ | |__|  __/\ V /  __/ | | |__| (_| | | || (_) | |
//    /____\___/|_| |_| |_|_.__/|_|\___| |___/ |_____\___| \_/ \___|_| |_____\__,_|_|\__\___/|_|
//
// This is a header file with all classes used by ZLE internally.
// SFML 2.6 is the main dependency here. Requires C++17.

#pragma once
#include <SFML/Graphics.hpp>
#include <map>
#include <vector>
#include <unordered_map>
#include <string>
#include <fstream>
#include <math.h>
#include <memory>
#include <random>
#include <filesystem>
#include <thread>
#include <mutex>

namespace zle
{
	constexpr const char* ZLE_VERSION = "#1.6";
	constexpr const char* TLM_VERSION = "TLM001";
	constexpr const char* PSY_VERSION = "PSY001";
	class TileMap : public sf::Transformable, public sf::Drawable
	{
		mutable sf::VertexArray arr;
		mutable sf::VertexBuffer buff;
		sf::Vector2u mapSize;
		sf::Vector2u tileSize;
		sf::Vector2u textureTileSize;
		sf::Color overAllColor;
		const sf::Texture* texture;
		unsigned short gapX, gapY;
		sf::Color mapColor;
		mutable sf::Uint8 updateBeforeDraw;
		std::vector<std::vector<unsigned short>> mapData;
		void ResizeBuffer()
		{
			//resize buffer
			mapData.resize(mapSize.x);
			for (int i = 0; i < mapSize.x; i++)
				mapData[i].resize(mapSize.y);
			arr.resize(6 * mapSize.x * mapSize.y);
			updateBeforeDraw |= 7;
		}
		void UpdatePositions() const
		{
			for (int i = 0; i < mapSize.x; i++)
				for (int j = 0; j < mapSize.y; j++)
				{
					sf::Vector2f position = sf::Vector2f(tileSize.x * i, tileSize.y * j);
					unsigned int v = (j + mapSize.y * i) * 6;

					arr[v + 0].position = position;
					arr[v + 1].position = position + sf::Vector2f(tileSize.x, 0);
					arr[v + 2].position = position + sf::Vector2f(tileSize);
					arr[v + 3].position = position + sf::Vector2f(0, tileSize.y);
					arr[v + 4].position = arr[v + 0].position;
					arr[v + 5].position = arr[v + 2].position;
				}
		}
		void UpdateColors() const
		{
			for (int i = 0; i < arr.getVertexCount(); i++)
				arr[i].color = overAllColor;
		}
		void UpdateTexCoord() const
		{
			unsigned short totalTilesInTextureX = UINT16_MAX;
			if (texture)
				totalTilesInTextureX = texture->getSize().x / (textureTileSize.x + gapX);
			for (int i = 0; i < mapSize.x; i++)
				for (int j = 0; j < mapSize.y; j++)
				{
					sf::Vector2f position = sf::Vector2f(tileSize.x * i, tileSize.y * j);
					unsigned int v = (j + mapSize.y * i) * 6;
					unsigned short TileX = mapData[i][j] % totalTilesInTextureX;
					unsigned short TileY = mapData[i][j] / totalTilesInTextureX;
					sf::FloatRect coord = sf::FloatRect(TileX * textureTileSize.x + TileX * gapX, TileY * textureTileSize.y + TileY * gapY, textureTileSize.x, textureTileSize.y);

					arr[v + 0].texCoords = sf::Vector2f(coord.left, coord.top);
					arr[v + 1].texCoords = sf::Vector2f(coord.left + coord.width, coord.top);
					arr[v + 2].texCoords = sf::Vector2f(coord.left + coord.width, coord.top + coord.height);
					arr[v + 3].texCoords = sf::Vector2f(coord.left, coord.top + coord.height);

					arr[v + 4].texCoords = arr[v + 0].texCoords;
					arr[v + 5].texCoords = arr[v + 2].texCoords;
				}
		}
		void DrawUpdate() const
		{
			//setup data
			if (updateBeforeDraw & 1)
				UpdatePositions();
			if (updateBeforeDraw & 2)
				UpdateColors();
			if (updateBeforeDraw & 4)
				UpdateTexCoord();
			//update
			if (sf::VertexBuffer::isAvailable() && arr.getVertexCount() > 0)
			{
				if (buff.getVertexCount() != arr.getVertexCount())
					buff.create(arr.getVertexCount());
				buff.update(&arr[0]);
			}
			updateBeforeDraw = 0;
		}
		template<typename T>
		static void Save(std::string& s, T var)
		{
			for (int i = 0; i < sizeof(T); i++)
				s += (reinterpret_cast<char*>(&var))[i];
		}
		template<typename T>
		static void Save(std::ofstream& s, T var)
		{
			for (int i = 0; i < sizeof(T); i++)
				s << (reinterpret_cast<char*>(&var))[i];
		}
		static void SaveStr(std::ofstream& s, const std::string& var)
		{
			sf::Uint32 size = var.size();
			for (int i = 0; i < sizeof(sf::Uint32); i++)
				s << (reinterpret_cast<char*>(&size))[i];
			s << var;
		}
		template<typename T>
		static void Load(sf::InputStream& input, T& t)
		{
			input.read(&t, sizeof(T));
		}
		static void LoadStr(sf::InputStream& input, std::string& data)
		{
			sf::Uint32 size;
			input.read(&size, sizeof(sf::Uint32));
			data.resize(size);
			input.read(&data[0], size);
		}
	public:

		/// <summary>
		/// Default constructor for a tilemap.
		/// </summary>
		TileMap()
			: texture(nullptr), gapX(0), gapY(0), updateBeforeDraw(0)
		{
			overAllColor = sf::Color::White;
			mapSize = sf::Vector2u(1, 1);
			tileSize = sf::Vector2u(1, 1);
			textureTileSize = sf::Vector2u(1, 1);
			arr.setPrimitiveType(sf::Triangles);
			if (sf::VertexBuffer::isAvailable())
			{
				buff.setPrimitiveType(sf::Triangles);
				buff.setUsage(sf::VertexBuffer::Dynamic);
			}
		}

		~TileMap() {}

		/// <summary>
		/// Creates a file on the system and saves the number
		/// of tiles in this tilemap as well as all of their
		/// tilemap IDs.
		/// </summary>
		/// <param name="fileName">File to save the data into</param>
		bool saveToFile(const std::filesystem::path& fileName)
		{
			std::ofstream save1;
			save1.open(fileName, std::ios::binary);
			return saveToFile(save1, fileName.extension() == ".tmap");
		}

		/// <summary>
		/// Saves the number of tiles in this tilemap
		/// as well as all of their tilemap IDs.
		/// </summary>
		/// <param name="save1">Output stream</param>
		bool saveToFile(std::ofstream& save1, bool legacy = false) const
		{
			if (save1.is_open())
			{
				std::string data = "";
				saveToMemory(data, legacy);
				save1 << data;
				return true;
			}
			return false;
		}

		/// <summary>
		/// Clears the string parameter and saves the number
		/// of tiles in this tilemap as well as all of their
		/// tilemap IDs.
		/// </summary>
		/// <param name="memory">String of memory to save the data into</param>
		void saveToMemory(std::string& memory, bool legacy = false) const
		{
			if (!legacy)
				memory += TLM_VERSION;
			Save(memory, getTileCount().x);
			Save(memory, getTileCount().y);
			for (int i = 0; i < getTileCount().x; i++)
				for (int j = 0; j < getTileCount().y; j++)
					Save(memory, getTileID(sf::Vector2u(i, j)));
		}

		/// <summary>
		/// Loads the tilemap from a file. This function creates
		/// a tilemap with the number of tiles as specified in
		/// the file and it sets all of the tile IDs.
		/// </summary>
		/// <param name="fileName">Path of the tilemap file to load</param>
		bool loadFromFile(const std::filesystem::path& fileName)
		{
			sf::FileInputStream load1;
			if (load1.open(fileName.string()))
				return loadFromStream(load1, fileName.extension() == ".tmap");
			return false;
		}

		/// <summary>
		/// Loads the tilemap from a a string. This function
		/// creates a tilemap with the number of tiles as
		/// specified in the file and it sets all of the tile IDs.
		/// </summary>
		/// <param name="data"></param>
		/// <param name="Size"></param>
		void loadFromMemory(const std::string& data, sf::Uint64 size, bool legacy = false)
		{
			sf::MemoryInputStream stream;
			stream.open(data.c_str(), size);
			loadFromStream(stream, legacy);
		}

		/// <summary>
		/// This function is used by loadFromFile() as well
		/// as loadFromMemory() to load tilemap data.
		/// </summary>
		/// <param name="stream">Stream to load from</param>
		bool loadFromStream(sf::InputStream& stream, bool legacy = false)
		{
			if (!legacy)
			{
				std::string ver = "123456";
				stream.read(&ver[0], 6);
				if (ver != TLM_VERSION)
					return false;
			}
			sf::Vector2u size;
			Load(stream, size.x);
			Load(stream, size.y);
			setTileCount(size);
			for (int i = 0; i < getTileCount().x; i++)
				for (int j = 0; j < getTileCount().y; j++)
				{
					unsigned short var;
					Load(stream, var);
					setTileID(sf::Vector2u(i, j), var);
				}
			return true;
		}

		/// <summary>
		/// Returns number of tiles per axis.
		/// </summary>
		/// <returns>Unsigned int, tiles per axis</returns>
		const sf::Vector2u& getTileCount() const
		{
			return mapSize;
		}

		/// <summary>
		/// Returns a texture used by the tilemap.
		/// </summary>
		/// <returns>Pointer to the texture</returns>
		const sf::Texture* getTexture() const
		{
			return texture;
		}

		/// <summary>
		/// Size of a single tile.
		/// </summary>
		/// <returns>Size of a tile</returns>
		const sf::Vector2u& getTileSize() const
		{
			return tileSize;
		}

		/// <summary>
		/// Get the numbers of pixels that are skipped per tile.
		/// </summary>
		/// <returns>Size of the texture gap in pixels</returns>
		const sf::Vector2u getTextureGap() const
		{
			return sf::Vector2u(gapX, gapY);
		}

		/// <summary>
		/// Get size of the portion in a texture that tiles use.
		/// </summary>
		/// <returns>Size of the texture that a tile uses in pixels</returns>
		const sf::Vector2u& getTextureTileSize() const
		{
			return textureTileSize;
		}

		/// <summary>
		/// Get the global color that is applied over tiles.
		/// </summary>
		/// <returns>Reference to the color</returns>
		const sf::Color& getColor() const
		{
			return overAllColor;
		}

		/// <summary>
		/// Get the ID of tile in the tilemap.
		/// </summary>
		/// <param name="tile">Coordinates of a tile</param>
		/// <returns>ID of the tile</returns>
		unsigned short getTileID(const sf::Vector2u& tile) const
		{
			return mapData[tile.x][tile.y];
		}

		/// <summary>
		/// Sets the total amount of tiles in a tilemap.
		/// </summary>
		/// <param name="mapSize">Size of the tilemap in tiles</param>
		void setTileCount(const sf::Vector2u& mapSize)
		{
			this->mapSize = mapSize;
			ResizeBuffer();
		}

		/// <summary>
		/// Sets the texture that this tilemap should use.
		/// </summary>
		/// <param name="texture">Pointer to the texture to use</param>
		void setTexture(const sf::Texture* texture)
		{
			this->texture = texture;
			updateBeforeDraw |= 4;
		}

		/// <summary>
		/// Sets the size of each tile.
		/// </summary>
		/// <param name="tileSize">Size of each tile</param>
		void setTileSize(const sf::Vector2u& tileSize)
		{
			this->tileSize = tileSize;
			updateBeforeDraw |= 1;
		}

		/// <summary>
		/// Texture gap is used to avoid texture bleeding which
		/// occurs when 2 tiles are right next to each other.
		/// To avoid this, a gap can exist between 2 tiles.
		/// </summary>
		/// <param name="gap">Gap size in pixels</param>
		void setTextureGap(unsigned short gapX, unsigned short gapY)
		{
			this->gapX = gapX;
			this->gapY = gapY;
			updateBeforeDraw |= 4;
		}

		/// <summary>
		/// Size of the texture each tile should use.
		/// </summary>
		/// <param name="textureTileSize">Size of the texture a tile uses.</param>
		void setTextureTileSize(const sf::Vector2u& textureTileSize)
		{
			this->textureTileSize = textureTileSize;
			updateBeforeDraw |= 4;
		}

		/// <summary>
		/// Sets a color that is multiplied with each
		/// tile when rendering. 
		/// </summary>
		/// <param name="color">Color that the tilemap should use.</param>
		void setColor(const sf::Color& color)
		{
			overAllColor = color;
			updateBeforeDraw |= 2;
		}

		/// <summary>
		/// This function changes which part of the texture
		/// a tile uses. ID 0 starts from the X = 0 and
		/// increases by TileSize.x. When it reaches the end, 
		/// X will reset to 0 and Y will increase by TileSize.y. 
		/// </summary>
		/// <param name="tile">Tile coordinates to change</param>
		/// <param name="ID">Tile ID to change it to</param>
		void setTileID(const sf::Vector2u& tile, unsigned short ID)
		{
			mapData[tile.x][tile.y] = ID;
			updateBeforeDraw |= 4;
		}
	private:

		virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const
		{
			states.transform *= getTransform();
			states.texture = texture;
			if (updateBeforeDraw)
				DrawUpdate();
			if (sf::VertexBuffer::isAvailable())
				target.draw(buff, states);
			else
				target.draw(arr, states);
		}
	};
	class ParticleSystemEvent
	{
	public:
		enum Type
		{
			OnUpdate = 1,
			OnDeath = 1 << 1,
			OnCreate = 1 << 2,
			All = OnUpdate | OnDeath | OnCreate,
			Count = 3
		};
		Type type = Type::All;
		sf::Vector2f particlePosition = sf::Vector2f();
		float particleRotation = 0;
		sf::Color particleColor = sf::Color();
		sf::Time lifeTime = sf::Time();
		ParticleSystemEvent() {}
		ParticleSystemEvent(const Type& type, const sf::Vector2f& particlePosition, const float& particleRotation, const sf::Color& particleColor, const sf::Time& lifeTime)
			: type(type), particlePosition(particlePosition), particleRotation(particleRotation), particleColor(particleColor), lifeTime(lifeTime)
		{
		}
		~ParticleSystemEvent() {}
	};
	class ParticleSystem : public sf::Drawable, public sf::Transformable
	{
	public:
		enum class ParticleType;
	private:
		struct Particle : public sf::Transformable
		{
			sf::Uint8 vertexCount;
			sf::Vertex* vertices;
			sf::Color currentColor;
			sf::Color startColor;
			sf::Color endColor;
			sf::Time lifeTime;
			sf::Time totalLifeTime;
			sf::Vector2f direction;
			sf::Vector2f offset;
			sf::Vector2f force;
			float startSize;
			float endSize;
			float startSpeed;
			float endSpeed;
			bool inUse;
			Particle()
				:inUse(0), offset(0, 0), direction(0, 0)
			{

			}
			void resetPointCoord()
			{
				switch (vertexCount)
				{
				case 1:
					vertices[0].texCoords = sf::Vector2f(0, 0);
				case 2: case 3:
				{
					float add = 360.0 / vertexCount;
					for (int i = 0; i < vertexCount; i++)
					{
						vertices[i].texCoords = sf::Vector2f(cos((add * i - 45) * 0.0174533), sin((add * i - 45) * 0.0174533));
					}
				}
				break;
				case 4:
					vertices[0].texCoords = sf::Vector2f(-1, -1);
					vertices[1].texCoords = sf::Vector2f(1, -1);
					vertices[2].texCoords = sf::Vector2f(1, 1);
					vertices[3].texCoords = sf::Vector2f(-1, 1);
					break;
				case 6:
					vertices[0].texCoords = sf::Vector2f(-1, -1);
					vertices[1].texCoords = sf::Vector2f(1, -1);
					vertices[2].texCoords = sf::Vector2f(1, 1);
					vertices[3].texCoords = sf::Vector2f(-1, 1);
					vertices[4].texCoords = vertices[0].texCoords;
					vertices[5].texCoords = vertices[2].texCoords;
					break;
				default:
					vertices[0].texCoords = sf::Vector2f(0, 0);
					break;
				}
			}
			const sf::Vector2f getPoint(int index)
			{
				switch (vertexCount)
				{
				case 1:
					return sf::Vector2f(0, 0);
				case 2: case 3: case 4:
				{
					float add = 360.0 / vertexCount;
					return sf::Vector2f(cos((add * index - 90) * 0.0174533), sin((add * index - 90) * 0.0174533));
					break;
				}
				case 6:
				{
					float add = 360.0 / 4;
					if (index == 4)
						return sf::Vector2f(cos((-90) * 0.0174533), sin((-90) * 0.0174533));
					if (index == 5)
						return sf::Vector2f(cos((add * 2 - 90) * 0.0174533), sin((add * 2 - 90) * 0.0174533));
					return sf::Vector2f(cos((add * index - 90) * 0.0174533), sin((add * index - 90) * 0.0174533));
					break;
				}
				}
				return sf::Vector2f();
			}
			void Update()
			{
				if (vertexCount == 1)
				{
					vertices[0].position = getPosition();
					vertices[0].color = currentColor;
				}
				else
					for (int i = 0; i < vertexCount; i++)
					{
						vertices[i].position = getTransform().transformPoint(getPoint(i));
						vertices[i].color = currentColor;
					}


			}
		};
		//event items
		void* spawnOnEvent[ParticleSystemEvent::Count];
		bool inherit[3][ParticleSystemEvent::Count];

		//random items
		std::mt19937 randomFunc;
		float r_lifeTime;
		float r_startSpeed;
		float r_endSpeed;
		float r_startSize;
		float r_endSize;
		sf::Color r_secondaryStart;
		sf::Color r_secondaryEnd;
		bool r_useSecondary;

		std::vector<ParticleSystemEvent> events;
		std::vector<sf::Vertex> arr;
		sf::VertexBuffer buff;
		std::vector<Particle> particles;
		sf::Time updateTime = sf::Time::Zero;
		sf::Time cntDown = sf::Time::Zero;
		mutable unsigned int totalParticles = 0;
		sf::Uint8 eventFlags = 0;

		int updateCalls = 0;
		//every particle variable
		sf::Vector2f startPos = sf::Vector2f();
		unsigned int maxParticles;
		float lifeSeconds;
		ParticleType type;
		float startRotation;
		float endRotation;
		unsigned int spawnCount;
		float spawnSeconds;
		float spawnRadius;
		sf::Vector2f constantForce;
		sf::Vector2f startForce;
		float startSize;
		float endSize;
		float startSpeed;
		float endSpeed;
		float fireAngle;
		float fireRotation;
		sf::Color startColor;
		sf::Color endColor;
		bool BothColors;
		float fading;
		bool randomStartRotation;
		bool createOnUpdate;
		const sf::Texture* texture;
		bool firstUpdate;
		bool keepUpWithFrameRate;
		bool drawNewestOnTop;
		sf::Time createFor = sf::Time::Zero;

		unsigned int verticesPerParticle() const
		{
			switch (type)
			{
			case ParticleSystem::ParticleType::Points:
				return 1;
				break;
			case ParticleSystem::ParticleType::Lines:
				return 2;
				break;
			case ParticleSystem::ParticleType::Triangles:
				return 3;
				break;
			case ParticleSystem::ParticleType::Quads:
				return 4;
				break;
			case ParticleSystem::ParticleType::QuadsTriangles:
				return 6;
				break;
			default:
				return 1;
				break;
			}
		}
		float getParam(const sf::Time& lifeTotal, const sf::Time& lifeLeft, float start, float end) const
		{
			float zeroToOne = (lifeTotal.asSeconds() - lifeLeft.asSeconds()) / lifeTotal.asSeconds();
			return (end - start) * zeroToOne + start;
		}
		void refreshPointer()
		{
			if (!drawNewestOnTop)
				return;
			for (int j = 0; j < particles.size(); j++)
			{
				particles[j].vertices = &arr[j * verticesPerParticle()];
			}
		}
		float getRandomValueTrig()
		{
			return static_cast<float>(randomFunc()) / randomFunc.max() * 6.283184f;
		}
		float getRandomValue01()
		{
			return static_cast<float>(randomFunc()) / randomFunc.max();
		}
		float getRandomValue11()
		{
			return static_cast<float>(randomFunc()) / (randomFunc.max() / 2) - 1;
		}
		int findFreeParticle()
		{
			for (int i = 0; i < particles.size(); i++)
			{
				if (!particles[i].inUse)
					return i;
			}
			return -1;
		}
		void setupArray()
		{
			deleteParticles();
			if (drawNewestOnTop)
				return;
			particles.clear();
			arr.clear();
			totalParticles = 0;
			particles.resize(maxParticles);
			arr.resize(maxParticles * verticesPerParticle());
			for (int i = 0; i < particles.size(); i++)
			{
				particles[i].vertexCount = verticesPerParticle();
				particles[i].vertices = &arr[i * verticesPerParticle()];
			}

		}
		void applyTexture(const sf::Texture* newTexture)
		{
			if (newTexture == nullptr)
				return;
			sf::Vector2f size = static_cast<sf::Vector2f>(newTexture->getSize());
			if (type == ParticleType::Points)
			{
				for (int i = 0; i < particles.size(); i++)
				{
					particles[i].vertices[0].texCoords = sf::Vector2f(size.x / 2, size.y / 2);
				}
			}
			else
				for (int i = 0; i < particles.size(); i++)
				{
					particles[i].resetPointCoord();
					for (int j = 0; j < particles[i].vertexCount; j++)
					{
						sf::Vector2f& v = particles[i].vertices[j].texCoords;
						v = sf::Vector2f((v.x + 1) / 2.0 * size.x, (v.y + 1) / 2.0 * size.y);
					}
				}

		}
		void ParticleEvents(ParticleSystemEvent evnt, ParticleSystem* target, int ID)
		{
			ParticleSystem& o = *reinterpret_cast<ParticleSystem*>(target->spawnOnEvent[ID]);
			sf::Vector2f startPos = o.getSpawnPosition();
			sf::Color startColor = o.getStartColor();
			float lifeTime = o.getLifeTime();
			if (target->inherit[0][ID])
				o.setSpawnPosition(evnt.particlePosition);
			if (target->inherit[1][ID])
				o.setStartColor(evnt.particleColor);
			if (target->inherit[2][ID])
				o.setLifeTime(evnt.lifeTime.asSeconds());
			o.Create();
			if (target->inherit[0][ID])
				o.setSpawnPosition(startPos);
			if (target->inherit[1][ID])
				o.setStartColor(startColor);
			if (target->inherit[2][ID])
				o.setLifeTime(lifeTime);
		}
		void createEvent(const ParticleSystemEvent::Type& type, const Particle& particle)
		{
			if ((eventFlags & (int)type) > 0)
				events.emplace_back(type, particle.getPosition(), particle.getRotation(), particle.currentColor, particle.lifeTime);
		}
		void deleteParticles()
		{
			particles.clear();
			arr.clear();
			totalParticles = 0;
		}

		void setup()
		{
			if (sf::VertexBuffer::isAvailable())
			{
				buff.setUsage(sf::VertexBuffer::Stream);
				if (type == ParticleType::QuadsTriangles)
					buff.setPrimitiveType(sf::PrimitiveType::Triangles);
				else
					buff.setPrimitiveType(static_cast<sf::PrimitiveType>(type));
			}
			setupArray();
		}
		template<typename T>
		static void Save(std::string& s, T var)
		{
			for (int i = 0; i < sizeof(T); i++)
				s += (reinterpret_cast<char*>(&var))[i];
		}
		template<typename T>
		static void Save(std::ofstream& s, T var)
		{
			for (int i = 0; i < sizeof(T); i++)
				s << (reinterpret_cast<char*>(&var))[i];
		}
		static void SaveStr(std::ofstream& s, const std::string& var)
		{
			sf::Uint32 size = var.size();
			for (int i = 0; i < sizeof(sf::Uint32); i++)
				s << (reinterpret_cast<char*>(&size))[i];
			s << var;
		}
		template<typename T>
		static void Load(sf::InputStream& input, T& t)
		{
			input.read(&t, sizeof(T));
		}
		static void LoadStr(sf::InputStream& input, std::string& data)
		{
			sf::Uint32 size;
			input.read(&size, sizeof(sf::Uint32));
			data.resize(size);
			input.read(&data[0], size);
		}
	public:
		enum class ParticleType
		{
			Points = sf::Points,
			Lines = sf::Lines,
			LineStrip = sf::LineStrip,
			Triangles = sf::Triangles,
			TriangleStrip = sf::TriangleStrip,
			TriangleFan = sf::TriangleFan,
			Quads = sf::Quads,
			QuadsTriangles
		};

		/// <summary>
		/// Default constructor for a particle system.
		/// </summary>
		ParticleSystem()
			: maxParticles(100), lifeSeconds(1), type(ParticleType::Triangles), startRotation(0), endRotation(0),
			spawnCount(1), spawnSeconds(0.1), constantForce(0, 0), startForce(0, 0), startSize(2), endSize(2),
			startSpeed(10), endSpeed(10), startColor(255, 255, 255, 255), endColor(255, 255, 255, 255), fading(1),
			randomStartRotation(0), createOnUpdate(1), texture(nullptr), firstUpdate(0), keepUpWithFrameRate(0),
			drawNewestOnTop(1), fireAngle(360.f), fireRotation(0), spawnRadius(0), r_lifeTime(0),
			r_endSize(0), r_startSize(0), r_startSpeed(0), r_endSpeed(0), r_useSecondary(0), r_secondaryEnd(255, 255, 255, 255), r_secondaryStart(255, 255, 255, 255),
			randomFunc(time(nullptr)), BothColors(true)
		{
			for (int i = 0; i < ParticleSystemEvent::Count; i++)
			{
				for (int j = 0; j < 3; j++)
					inherit[j][i] = 0;
				spawnOnEvent[i] = nullptr;
			}
			setup();
		}

		~ParticleSystem()
		{
		}

		/// <summary>
		/// Saves the particle system to a new file.
		/// </summary>
		/// <param name="fileName">File to save to</param>
		bool saveToFile(const std::filesystem::path& fileName) const
		{
			std::ofstream save1;
			save1.open(fileName, std::ios::binary);
			return saveToFile(save1, fileName.extension() == ".part");
		}

		/// <summary>
		/// Saves the particle system to an existing open file.
		/// </summary>
		/// <param name="save1">Stream to save to</param>
		bool saveToFile(std::ofstream& save1, bool legacy = false) const
		{
			if (save1.is_open())
			{
				std::string data = "";
				saveToMemory(data);
				save1 << data;
				return true;
			}
			return false;
		}

		/// <summary>
		/// Saves the particle system to existing memory.
		/// </summary>
		/// <param name="save1">String to save to</param>
		void saveToMemory(std::string& memory, bool legacy = false) const
		{
			if (!legacy)
				memory += PSY_VERSION;
			Save(memory, getConstantForce());
			Save(memory, getEndColor());
			Save(memory, getEndRotation());
			Save(memory, getEndSize());
			Save(memory, getEndSpeed());
			Save(memory, getFading());
			Save(memory, getLifeTime());
			Save(memory, getMaxParticles());
			Save(memory, getParticleType());
			Save(memory, getRandomStartRotation());
			Save(memory, getSpawnAngle());
			Save(memory, getSpawnCount());
			Save(memory, getSpawnRate());
			Save(memory, getSpawnRotation());
			Save(memory, getStartColor());
			Save(memory, getStartForce());
			Save(memory, getStartRotation());
			Save(memory, getStartSize());
			Save(memory, getStartSpeed());
			Save(memory, getDrawNewestOnTop());
			Save(memory, getKeepUpWithFrameRate());
			Save(memory, getSpawnPosition());
			Save(memory, r_lifeTime);
			Save(memory, r_secondaryEnd);
			Save(memory, r_secondaryStart);
			Save(memory, r_endSize);
			Save(memory, r_startSize);
			Save(memory, r_endSpeed);
			Save(memory, r_startSpeed);
			Save(memory, getSpawnRadius());
		}

		/// <summary>
		/// Loads the particle system from the specified stream.
		/// Used by loadFromFile().
		/// </summary>
		/// <param name="file">Stream to load from</param>
		bool loadFromStream(sf::InputStream& stream, bool legacy = false)
		{
			if (!legacy)
			{
				std::string ver = "123456";
				stream.read(&ver[0], 6);
				if (ver != PSY_VERSION)
					return false;
			}
			Load(stream, constantForce);
			Load(stream, endColor);
			Load(stream, endRotation);
			Load(stream, endSize);
			Load(stream, endSpeed);
			Load(stream, fading);
			Load(stream, lifeSeconds);
			Load(stream, maxParticles);
			Load(stream, type);
			Load(stream, randomStartRotation);
			Load(stream, fireAngle);
			Load(stream, spawnCount);
			Load(stream, spawnSeconds);
			Load(stream, fireRotation);
			Load(stream, startColor);
			Load(stream, startForce);
			Load(stream, startRotation);
			Load(stream, startSize);
			Load(stream, startSpeed);
			Load(stream, drawNewestOnTop);
			Load(stream, keepUpWithFrameRate);
			Load(stream, startPos);
			Load(stream, r_lifeTime);
			Load(stream, r_secondaryEnd);
			Load(stream, r_secondaryStart);
			Load(stream, r_endSize);
			Load(stream, r_startSize);
			Load(stream, r_endSpeed);
			Load(stream, r_startSpeed);
			Load(stream, spawnRadius);
			setParticleType(type);
			return true;
		}

		/// <summary>
		/// Loads the particle system from a file on the disk. Loads all 
		/// state variables from the file. This function does not check if 
		/// the particle system was exported in the correct version and 
		/// therefore may crash if there is a mismatch.
		/// </summary>
		/// <param name="fileName">File to load from</param>
		bool loadFromFile(const std::filesystem::path& fileName)
		{
			sf::FileInputStream load1;
			if (load1.open(fileName.string()))
				return loadFromStream(load1, fileName.extension() == ".part");
			return false;
		}

		/// <summary>
		/// Returns the angle (direction) in which particles may spawn.
		/// </summary>
		/// <returns>Reference to the angle</returns>
		const float& getSpawnAngle() const
		{
			return fireAngle;
		}

		/// <summary>
		/// Returns the rotation in which particles may spawn.
		/// </summary>
		/// <returns>Reference to the rotation</returns>
		const float& getSpawnRotation() const
		{
			return fireRotation;
		}

		/// <summary>
		/// Returns the maximum allowed number of particles.
		/// </summary>
		/// <returns>Reference to max particles</returns>
		const unsigned int& getMaxParticles() const
		{
			return maxParticles;
		}

		/// <summary>
		/// Returns the base life time of a particle.
		/// </summary>
		/// <returns>Reference to life time</returns>
		const float& getLifeTime() const
		{
			return lifeSeconds;
		}

		/// <summary>
		/// Returns the primitive type of the particle.
		/// </summary>
		/// <returns>Reference to primitive type</returns>
		const ParticleType& getParticleType() const
		{
			return type;
		}

		/// <summary>
		/// Returns the starting rotation of a particle.
		/// </summary>
		/// <returns>Reference to the starting rotation</returns>
		const float& getStartRotation() const
		{
			return startRotation;
		}

		/// <summary>
		/// Returns the ending rotation of a particle.
		/// </summary>
		/// <returns>Reference to the ending rotation</returns>
		const float& getEndRotation() const
		{
			return endRotation;
		}

		/// <summary>
		/// Returns how many particles are spawned on each Create().
		/// </summary>
		/// <returns>Reference to the spawn amount</returns>
		const unsigned int& getSpawnCount() const
		{
			return spawnCount;
		}

		/// <summary>
		/// Returns how frequently are particles spawned.
		/// </summary>
		/// <returns>Reference to the spawning time in seconds</returns>
		const float& getSpawnRate() const
		{
			return spawnSeconds;
		}

		/// <summary>
		/// Returns the constant force that adds up every frame.
		/// </summary>
		/// <returns>Reference to the constant force</returns>
		const sf::Vector2f& getConstantForce() const
		{
			return constantForce;
		}

		/// <summary>
		/// Returns the start force that gets applied when a particle is created.
		/// </summary>
		/// <returns>Reference to the constant force</returns>
		const sf::Vector2f& getStartForce() const
		{
			return startForce;
		}

		/// <summary>
		/// Returns the start size of a particle.
		/// </summary>
		/// <returns>Reference to the start size</returns>
		const float& getStartSize() const
		{
			return startSize;
		}

		/// <summary>
		/// Returns the end size of a particle.
		/// </summary>
		/// <returns>Reference to the end size</returns>
		const float& getEndSize() const
		{
			return endSize;
		}

		/// <summary>
		/// Returns the start speed of a particle.
		/// </summary>
		/// <returns>Reference to the start speed</returns>
		const float& getStartSpeed() const
		{
			return startSpeed;
		}

		/// <summary>
		/// Returns the end speed of a particle.
		/// </summary>
		/// <returns>Reference to the end speed</returns>
		const float& getEndSpeed() const
		{
			return endSpeed;
		}

		/// <summary>
		/// Returns the start color of a particle.
		/// </summary>
		/// <returns>Reference to the start color</returns>
		const sf::Color& getStartColor() const
		{
			return startColor;
		}

		/// <summary>
		/// Returns the end color of a particle.
		/// </summary>
		/// <returns>Reference to the end color</returns>
		const sf::Color& getEndColor() const
		{
			return endColor;
		}

		/// <summary>
		/// Returns when the particle starts fading in it's life time.
		/// Value from 0 (at 0%) to 1 (at 100%).
		/// </summary>
		/// <returns>Reference to fading</returns>
		const float& getFading() const
		{
			return fading;
		}

		/// <summary>
		/// Returns if the particle should be randomly rotated when it spawns.
		/// </summary>
		/// <returns>True if on, false otherwise</returns>
		const bool& getRandomStartRotation() const
		{
			return randomStartRotation;
		}

		/// <summary>
		/// Returns a pointer to the texture. If none is used, returns a nullptr.
		/// </summary>
		/// <returns>Pointer to the texture used</returns>
		const sf::Texture* getTexture() const
		{
			return texture;
		}

		/// <summary>
		/// Returns a number of particles that are currently active.
		/// </summary>
		/// <returns>Reference to the number of particles active</returns>
		const unsigned int& getActiveParticles() const
		{
			if (drawNewestOnTop)
				totalParticles = particles.size();
			return totalParticles;
		}

		/// <summary>
		/// Returns the offset from the position of the particle system
		/// where particles should be spawned.
		/// </summary>
		/// <returns>Reference to the particle spawning position</returns>
		const sf::Vector2f& getSpawnPosition() const
		{
			return startPos;
		}

		/// <summary>
		/// Returns the radius from the spawn position where particles can be spawned.
		/// </summary>
		/// <returns>Reference to the particle spawning radius</returns>
		const float& getSpawnRadius() const
		{
			return spawnRadius;
		}

		/// <summary>
		/// Returns the tolerance for particle life time.
		/// </summary>
		/// <returns>Reference to the random life time</returns>
		const float& getRandomLifeTime() const
		{
			return r_lifeTime;
		}

		/// <summary>
		/// Returns the tolerance for particle start speed.
		/// </summary>
		/// <returns>Reference to the random start speed</returns>
		const float& getRandomStartSpeed() const
		{
			return r_startSpeed;
		}

		/// <summary>
		/// Returns the tolerance for particle end speed.
		/// </summary>
		/// <returns>Reference to the random start speed</returns>
		const float& getRandomEndSpeed() const
		{
			return r_endSpeed;
		}

		/// <summary>
		/// Returns the tolerance for particle start size.
		/// </summary>
		/// <returns>Reference to the random start size</returns>
		const float& getRandomStartSize() const
		{
			return r_startSize;
		}

		/// <summary>
		/// Returns the tolerance for particle end size.
		/// </summary>
		/// <returns>Reference to the random end size</returns>
		const float& getRandomEndSize() const
		{
			return r_endSize;
		}

		/// <summary>
		/// Returns the second start color of particles.
		/// Used for random particle color selection.
		/// </summary>
		/// <returns>Reference to the second start color</returns>
		const sf::Color& getSecondStartColor() const
		{
			return r_secondaryStart;
		}

		/// <summary>
		/// Returns the second end color of particles.
		/// Used for random particle color selection.
		/// </summary>
		/// <returns>Reference to the second end color</returns>
		const sf::Color& getSecondEndColor() const
		{
			return r_secondaryEnd;
		}

		/// <summary>
		/// Adds or removes flags for capturing.
		/// </summary>
		/// <param name="state">Add or remove flags</param>
		/// <param name="type">Flags to add/remove</param>
		void setCaptureEvent(bool state, sf::Uint8 type = static_cast<sf::Uint8>(ParticleSystemEvent::All))
		{
			if (state)
			{
				eventFlags |= type;
			}
			else
			{
				eventFlags &= ~type;
			}
		}

		/// <summary>
		/// Returns the particle rendering system.
		/// </summary>
		/// <returns>True if drawing newest on top, false otherwise</returns>
		bool getDrawNewestOnTop() const
		{
			return drawNewestOnTop;
		}

		/// <summary>
		/// Returns the framerate sync settings.
		/// </summary>
		/// <returns>True if on, false otherwise</returns>
		bool getKeepUpWithFrameRate() const
		{
			return keepUpWithFrameRate;
		}

		/// <summary>
		/// Checks if there is an event to process, in which
		/// case the event gets returned.
		/// </summary>
		/// <returns>True if an event is available for processing</returns>
		bool pollEvent(ParticleSystemEvent& event)
		{
			if (events.size() > 0)
			{
				event = events.back();
				events.pop_back();
				return 1;
			}
			return 0;
		}

		/// <summary>
		/// Returns a read only reference to the vector of events.
		/// </summary>
		/// <returns>Reference to the array of particle system events</returns>
		const std::vector<ParticleSystemEvent>& getEvents() const
		{
			return events;
		}

		/// <summary>
		/// Deletes all currently stored events.
		/// </summary>
		void clearEvents()
		{
			events.clear();
		}

		/// <summary>
		/// Selects whether particles should gradually change colors or not.
		/// If true, particles will change their color over their lifetime.
		/// </summary>
		/// <param name="state">True if particles should change colors, false otherwise</param>
		void useBothColors(bool state = true)
		{
			BothColors = state;
		}

		/// <summary>
		/// Selects the particle rendering and management system.
		/// If true, particles will be stored in a constantly resizing
		/// buffer which on a first in first out system. This will
		/// allow newest particles to be drawn at the top.
		/// If false, particles will be stored in a buffer that is
		/// constant in size and newest particles are not guaranteed to
		/// be drawn on top. This option can increase performance when
		/// dealing with very large particle counts or OnUpdate particle
		/// events.
		/// </summary>
		/// <param name="state">True for fifo system, false otherwise</param>
		void setDrawNewestOnTop(bool state = true)
		{
			drawNewestOnTop = state;
			setupArray();
			applyTexture(texture);
		}

		/// <summary>
		/// Specifies the rotation range in which particles should be fired.
		/// By default is set to 360.
		/// </summary>
		/// <param name="fireAngle">Range for particles to be fired in</param>
		void setSpawnAngle(float fireAngle = 360.f)
		{
			this->fireAngle = fireAngle;
		}

		/// <summary>
		/// Specifies the offset from the particle system position
		/// where particles should be spawned.
		/// </summary>
		/// <param name="pos">Offset from the particle system position</param>
		void setSpawnPosition(const sf::Vector2f& pos)
		{
			this->startPos = pos;
		}

		/// <summary>
		/// Specifies the rotation from the particle system spawning position
		/// where particles should be fired.
		/// </summary>
		/// <param name="rotation">Rotation of the firing system</param>
		void setSpawnRotation(float rotation)
		{
			this->fireRotation = rotation;
		}

		/// <summary>
		/// Specifies the maximum number of particles that can exist
		/// at the same time in this particle system.
		/// </summary>
		/// <param name="count">Maximum number of particles</param>
		void setMaxParticles(unsigned int count)
		{
			maxParticles = count;
			setupArray();
			applyTexture(texture);
		}

		/// <summary>
		/// Specifies how long each particle is active.
		/// </summary>
		/// <param name="seconds">Time each particle is active in seconds</param>
		void setLifeTime(float seconds)
		{
			lifeSeconds = seconds;
		}

		/// <summary>
		/// Extra life time which is added to the base life time. 
		/// It can be at minimum -seconds and at maximum +seconds.
		/// </summary>
		/// <param name="seconds">Extra life time offset</param>
		void setRandomLifeTime(float seconds)
		{
			r_lifeTime = seconds;
		}

		/// <summary>
		/// Extra speed which is added to the base start speed. 
		/// It can be at minimum -speed and at maximum +speed.
		/// </summary>
		/// <param name="speed">Extra start speed offset</param>
		void setRandomStartSpeed(float speed)
		{
			r_startSpeed = speed;
		}

		/// <summary>
		/// Extra speed which is added to the base end speed. 
		/// It can be at minimum -speed and at maximum +speed.
		/// </summary>
		/// <param name="speed">Extra end speed offset</param>
		void setRandomEndSpeed(float speed)
		{
			r_endSpeed = speed;
		}

		/// <summary>
		/// Extra size which is added to the base start size. 
		/// It can be at minimum -size and at maximum +size.
		/// </summary>
		/// <param name="size">Extra start size offset</param>
		void setRandomStartSize(float size)
		{
			r_startSize = size;
		}

		/// <summary>
		/// Extra size which is added to the base end size. 
		/// It can be at minimum -size and at maximum +size.
		/// </summary>
		/// <param name="size">Extra end size offset</param>
		void setRandomEndSize(float size)
		{
			r_endSize = size;
		}

		/// <summary>
		/// Specifies if second colors should be used. 
		/// When second colors are active, a random color is picked between
		/// the base color and the second color, for example: base color is
		/// pure red and the second color is pure yellow. Colors that might
		/// get randomly picked will have the R channel set at 255 since both
		/// the base and second color have that channel at 255. Similarly,
		/// the B channel will be set to 0. Only the G channel will be randomly
		/// picked between 0 and 255.
		/// </summary>
		/// <param name="state">True for using second colors, false otherwise</param>
		void useSecondColors(bool state = false)
		{
			r_useSecondary = state;
		}

		/// <summary>
		/// Specifies the second start color for random color picking.
		/// See useSecondColors() for more info.
		/// </summary>
		/// <param name="color">Second start color to use</param>
		void setSecondStartColor(const sf::Color& color)
		{
			r_secondaryStart = color;
		}

		/// <summary>
		/// Specifies the second end color for random color picking.
		/// See useSecondColors() for more info.
		/// </summary>
		/// <param name="color">Second end color to use</param>
		void setSecondEndColor(const sf::Color& color)
		{
			r_secondaryEnd = color;
		}

		/// <summary>
		/// Specifies the particle primitive type used.
		/// If QuadsTriangles is selected, particle system
		/// will internally use triangles but create quads
		/// with 2 triangles to allow for OpenGL ES support
		/// which does not support quads as a primitive type.
		/// </summary>
		/// <param name="type">Primitive type to use</param>
		void setParticleType(ParticleType type)
		{
			this->type = type;
			if (sf::VertexBuffer::isAvailable())
			{
				if (type == ParticleType::QuadsTriangles)
					buff.setPrimitiveType(sf::PrimitiveType::Triangles);
				else
					buff.setPrimitiveType((sf::PrimitiveType)type);
			}
			setupArray();
			applyTexture(texture);
		}

		/// <summary>
		/// Specifies the starting rotation of particles.
		/// </summary>
		/// <param name="rotation">Starting rotation of particles</param>
		void setStartRotation(float rotation)
		{
			startRotation = rotation;
		}

		/// <summary>
		/// Specifies the ending rotation of particles.
		/// </summary>
		/// <param name="rotation">Ending rotation of particles</param>
		void setEndRotation(float rotation)
		{
			endRotation = rotation;
		}

		/// <summary>
		/// Specifies the number of particles that will get created when
		/// Create() is called.
		/// </summary>
		/// <param name="count">Number of particles to be spawned on Create()</param>
		void setSpawnCount(unsigned int count = 1)
		{
			spawnCount = count;
		}

		/// <summary>
		/// Specifies how frequently are particles spawned.
		/// </summary>
		/// <param name="time">Time between spawning particles in seconds</param>
		void setSpawnRate(float time)
		{
			spawnSeconds = time;
			cntDown = sf::Time::Zero;
			createFor = sf::Time::Zero;
		}

		/// <summary>
		/// Specifies a constant force that adds up to the direction every frame.
		/// </summary>
		/// <param name="force">The constant force to apply</param>
		void setConstantForce(const sf::Vector2f& force)
		{
			constantForce = force;
		}

		/// <summary>
		/// Specifies a start force that gets applied on particle creation.
		/// </summary>
		/// <param name="force">The start force to apply</param>
		void setStartForce(const sf::Vector2f& force)
		{
			startForce = force;
		}

		/// <summary>
		/// Specifies a size that a particle has when it is created.
		/// </summary>
		/// <param name="size">The start size</param>
		void setStartSize(float size)
		{
			startSize = size;
		}

		/// <summary>
		/// Specifies a size that a particle has when it dies.
		/// If start size is different than the end size,
		/// it will slowly shift from the start to the end size.
		/// </summary>
		/// <param name="size">The end size</param>
		void setEndSize(float size)
		{
			endSize = size;
		}

		/// <summary>
		/// Specifies a speed that a particle has when it is created.
		/// </summary>
		/// <param name="speed">The start speed</param>
		void setStartSpeed(float speed)
		{
			startSpeed = speed;
		}

		/// <summary>
		/// Specifies a speed that a particle has when it dies.
		/// If start speed is different than the end speed,
		/// it will slowly shift from the start to the end speed.
		/// </summary>
		/// <param name="speed">The end speed</param>
		void setEndSpeed(float speed)
		{
			endSpeed = speed;
		}

		/// <summary>
		/// Specifies a color that a particle has when it is created.
		/// </summary>
		/// <param name="color">The start color</param>
		void setStartColor(const sf::Color& color)
		{
			startColor = color;
		}

		/// <summary>
		/// Specifies a color that a particle has when it dies.
		/// If start color is different than the end color,
		/// it will slowly shift from the start to the end color.
		/// </summary>
		/// <param name="color">The end color</param>
		void setEndColor(const sf::Color& color)
		{
			endColor = color;
		}

		/// <summary>
		/// Specifies when the particle should start becoming transparent
		/// during it's life time. Valid values are from 0 to 1.
		/// If 0, it will start fading from the moment it is created.
		/// If 0.5, it will start fading after half of it's life time has passed.
		/// If 1, fading will be instant.
		/// </summary>
		/// <param name="value">Value from 0 to 1</param>
		void setFading(float value)
		{
			fading = value;
		}

		/// <summary>
		/// Specifies if particles should be randomly rotated when they spawn.
		/// This option disables rotation of particles over life time.
		/// </summary>
		/// <param name="state">True for random starting rotation, false otherwise</param>
		void setRandomStartRotation(bool state = false)
		{
			randomStartRotation = state;
		}

		/// <summary>
		/// Specifies a texture for particles to use. Pass nullptr to
		/// disable textures.
		/// </summary>
		/// <param name="texture">Texture for particles to use</param>
		void setTexture(const sf::Texture* texture)
		{
			applyTexture(texture);
			this->texture = texture;
		}

		/// <summary>
		/// Specifies whether the update function can spawn particles.
		/// If true, particles will spawn in regular intervals as
		/// specified by setSpawnRate().
		/// If false, only outside calls to Create() and CreateFor()
		/// will spawn particles.
		/// </summary>
		/// <param name="createOnUpdate">True if Update() can spawn particles, false otherwise</param>
		void setCreateOnUpdate(bool createOnUpdate)
		{
			this->createOnUpdate = createOnUpdate;
		}

		/// <summary>
		/// Specifies if particle system should spawn extra particles in
		/// case of a ex. lag spike.
		/// If true, the particle system will spawn particles that couldn't
		/// be spawned due to the lack of updates.
		/// If false, particle system won't care about such lacks of updates.
		/// </summary>
		/// <param name="state">True for spawning extra particles, false otherwise</param>
		void setKeepUpWithFrameRate(bool state)
		{
			keepUpWithFrameRate = state;
			cntDown = sf::Time::Zero;
			createFor = sf::Time::Zero;
		}

		/// <summary>
		/// Specify a radius from the spawn position where the particles can be spawned.
		/// </summary>
		void setSpawnRadius(float radius)
		{
			spawnRadius = radius;
		}

		/// <summary>
		/// Deletes all particles.
		/// </summary>
		void Clear()
		{
			for (int i = 0; i < particles.size(); i++)
			{
				if (particles[i].inUse)
					particles[i].lifeTime = sf::Time::Zero;
			}
			Update(sf::Time::Zero);
		}

		/// <summary>
		/// Specifies how long should new particles be created from now.
		/// Has no effect if createOnUpdate is true.
		/// </summary>
		/// <param name="time">How long to spawn particles for</param>
		void CreateFor(const sf::Time& time)
		{
			createFor = time;
			Create();
		}

		/// <summary>
		/// Spawns a new particle if there is room for it in the buffer.
		/// </summary>
		void Create()
		{
			if (drawNewestOnTop)
				for (int j = 0; j < spawnCount; j++)
				{
					if (arr.size() / verticesPerParticle() >= maxParticles)
						return;
					Particle particle;
					particle.vertexCount = verticesPerParticle();
					for (int i = 0; i < verticesPerParticle(); i++)
						arr.push_back(sf::Vertex());
					particle.vertices = &arr[arr.size() - verticesPerParticle()];
					particle.lifeTime = sf::seconds(lifeSeconds + getRandomValue11() * r_lifeTime);
					particle.totalLifeTime = particle.lifeTime;
					sf::Color startRandom;
					if (r_useSecondary)
						startRandom = sf::Color(startColor.r + getRandomValue01() * (r_secondaryStart.r - startColor.r),
							startColor.g + getRandomValue01() * (r_secondaryStart.g - startColor.g),
							startColor.b + getRandomValue01() * (r_secondaryStart.b - startColor.b));
					else
						startRandom = startColor;

					sf::Color endRandom;
					if (r_useSecondary)
						endRandom = sf::Color(endColor.r + getRandomValue01() * (r_secondaryEnd.r - endColor.r),
							endColor.g + getRandomValue01() * (r_secondaryEnd.g - endColor.g),
							endColor.b + getRandomValue01() * (r_secondaryEnd.b - endColor.b));
					else
						endRandom = endColor;
					particle.currentColor = startRandom;
					particle.startColor = startRandom;
					particle.endColor = endRandom;
					particle.force = startForce;
					particle.inUse = 1;
					particle.startSize = startSize + getRandomValue11() * r_startSize;
					particle.endSize = endSize + getRandomValue11() * r_endSize;
					particle.startSpeed = startSpeed + getRandomValue11() * r_startSpeed;
					particle.endSpeed = endSpeed + getRandomValue11() * r_endSpeed;
					float randomDirection = getRandomValue01() * fireAngle + fireRotation;
					particle.direction = sf::Vector2f(cos(randomDirection * 0.0174533), sin(randomDirection * 0.0174533));
					particle.setRotation(randomStartRotation ? getRandomValue01() * 360 : 0);
					if (texture != nullptr)
					{
						sf::Vector2f size = (sf::Vector2f)texture->getSize();
						particle.resetPointCoord();
						for (int j = 0; j < particle.vertexCount; j++)
						{
							sf::Vector2f& v = particle.vertices[j].texCoords;
							v = sf::Vector2f((v.x + 1) / 2.0 * size.x, (v.y + 1) / 2.0 * size.y);
						}
					}
					float random1 = getRandomValueTrig();
					float random2 = getRandomValue01();
					sf::Vector2f randomPos = startPos + sf::Vector2f(
						cos(random1) * random2 * spawnRadius,
						sin(random1) * random2 * spawnRadius);
					particle.offset = randomPos;
					particle.setPosition(randomPos);
					totalParticles++;
					particle.Update();
					particles.push_back(particle);
					createEvent(ParticleSystemEvent::Type::OnCreate, particle);
				}
			else
			{
				for (int j = 0; j < spawnCount; j++)
				{

					int id = findFreeParticle();

					if (id == -1)
						return;
					Particle& particle = particles[id];
					particle.lifeTime = sf::seconds(lifeSeconds + getRandomValue11() * r_lifeTime);
					particle.totalLifeTime = particle.lifeTime;
					sf::Color startRandom;
					if (r_useSecondary)
						startRandom = sf::Color(startColor.r + getRandomValue01() * (r_secondaryStart.r - startColor.r),
							startColor.g + getRandomValue01() * (r_secondaryStart.g - startColor.g),
							startColor.b + getRandomValue01() * (r_secondaryStart.b - startColor.b));
					else
						startRandom = startColor;

					sf::Color endRandom;
					if (r_useSecondary)
						endRandom = sf::Color(endColor.r + getRandomValue01() * (r_secondaryEnd.r - endColor.r),
							endColor.g + getRandomValue01() * (r_secondaryEnd.g - endColor.g),
							endColor.b + getRandomValue01() * (r_secondaryEnd.b - endColor.b));
					else
						endRandom = endColor;
					particle.currentColor = startRandom;
					particle.startColor = startRandom;
					particle.endColor = endRandom;
					particle.force = startForce;
					particle.inUse = 1;
					particle.startSize = startSize + getRandomValue11() * r_startSize;
					particle.endSize = endSize + getRandomValue11() * r_endSize;
					particle.startSpeed = startSpeed + getRandomValue11() * r_startSpeed;
					particle.endSpeed = endSpeed + getRandomValue11() * r_endSpeed;

					float randomDirection = getRandomValue01() * fireAngle + fireRotation;
					particle.direction = sf::Vector2f(cos(randomDirection * 0.0174533), sin(randomDirection * 0.0174533));
					particle.setRotation(randomStartRotation ? getRandomValue01() * 360 : 0);
					float random1 = getRandomValueTrig();
					float random2 = getRandomValue01();
					sf::Vector2f randomPos = startPos + sf::Vector2f(
						cos(random1) * random2 * spawnRadius,
						sin(random1) * random2 * spawnRadius);
					particle.offset = randomPos;
					particle.setPosition(randomPos);
					particle.Update();
					totalParticles++;

					createEvent(ParticleSystemEvent::Type::OnCreate, particle);
				}
			}
			refreshPointer();
			return;

		}

		/// <summary>
		/// Updates the particle system. This function is expensive
		/// and should be called once per frame. This function
		/// automatically generates new particles if createOnUpdate
		/// is true. Particle events are also generated if they are
		/// used.
		/// </summary>
		/// <param name="deltaTime">Time since last frame</param>
		void Update(const sf::Time& deltaTime)
		{
			sf::Clock clock;
			if (createOnUpdate)
			{
				cntDown -= deltaTime;
				if (cntDown.asSeconds() <= 0)
				{
					Create();
					cntDown += sf::seconds(spawnSeconds);
					if (!keepUpWithFrameRate)
						while (cntDown.asSeconds() <= 0)
							cntDown += sf::seconds(spawnSeconds);
				}

			}
			else
			{
				createFor -= deltaTime;
				if (createFor.asSeconds() > 0)
				{
					cntDown -= deltaTime;
					if (cntDown.asSeconds() <= 0)
					{
						Create();
						cntDown += sf::seconds(spawnSeconds);
						if (!keepUpWithFrameRate)
							while (cntDown.asSeconds() <= 0)
								cntDown += sf::seconds(spawnSeconds);
					}
				}
			}
			for (int i = 0; i < particles.size(); i++)
			{
				if (!particles[i].inUse)
					continue;
				//update
				Particle& p = particles[i];
				p.lifeTime -= deltaTime;
			}
			int cnt = 0;
			if (drawNewestOnTop)
			{
				cnt = 0;
				for (int i = 0; i < particles.size(); i++)
				{
					if (!particles[i].inUse)
						continue;
					Particle& p = particles[i];
					if (p.lifeTime.asSeconds() <= 0)
					{
						cnt++;
						createEvent(ParticleSystemEvent::Type::OnDeath, p);
						p.currentColor = sf::Color::Transparent;
						p.Update();
					}
					else
						break;
				}
				if (cnt > 0)
				{
					//kill particle
					for (int j = 0; j < verticesPerParticle(); j++)
						arr.erase(arr.begin(), arr.begin() + cnt);
					particles.erase(particles.begin(), particles.begin() + cnt);
					totalParticles -= cnt;
					if (totalParticles < 0)
						totalParticles = 0;
					refreshPointer();
				}
			}
			else
			{
				for (int i = 0; i < particles.size(); i++)
				{
					if (!particles[i].inUse)
						continue;
					Particle& p = particles[i];
					if (p.lifeTime.asSeconds() <= 0)
					{
						createEvent(ParticleSystemEvent::Type::OnDeath, p);
						p.currentColor = sf::Color::Transparent;
						p.Update();
						totalParticles--;
						if (totalParticles < 0)
							totalParticles = 0;
						p.inUse = 0;
					}
				}
			}
			//update
			for (int i = 0; i < particles.size(); i++)
			{
				if (!particles[i].inUse)
					continue;
				Particle& p = particles[i];
				p.force += constantForce * deltaTime.asSeconds();
				p.offset += (p.direction + p.force) * deltaTime.asSeconds() * getParam(p.totalLifeTime, p.lifeTime, p.startSpeed, p.endSpeed);
				p.setPosition(p.offset);

				if (!randomStartRotation)
					p.setRotation(getParam(p.totalLifeTime, p.lifeTime, startRotation, endRotation));
				p.setScale(sf::Vector2f(1, 1) * getParam(p.totalLifeTime, p.lifeTime, p.startSize, p.endSize));

				float zeroToOne = std::min((p.totalLifeTime.asSeconds() - p.lifeTime.asSeconds()) / p.totalLifeTime.asSeconds(), 1.f);
				float fadingT = p.currentColor.a;
				if (zeroToOne > fading && fading < 1)
				{
					zeroToOne -= fading;
					zeroToOne /= (1 - fading);
					fadingT = 255 - 255 * zeroToOne;
				}
				if (BothColors)
				{
					p.currentColor = sf::Color(getParam(p.totalLifeTime, p.lifeTime, p.startColor.r, p.endColor.r),
						getParam(p.totalLifeTime, p.lifeTime, p.startColor.g, p.endColor.g),
						getParam(p.totalLifeTime, p.lifeTime, p.startColor.b, p.endColor.b), fadingT);
				}
				else
				{
					p.currentColor.a = fadingT;
				}
				createEvent(ParticleSystemEvent::Type::OnUpdate, p);
			}
			for (int i = 0; i < particles.size(); i++)
			{
				if (!particles[i].inUse)
					continue;
				Particle& p = particles[i];
				p.Update();
			}

			if (arr.size() > 0 && sf::VertexBuffer::isAvailable())
			{
				if (buff.getVertexCount() != arr.size())
					buff.create(arr.size());
				buff.update(&arr[0]);
			}
			updateTime = clock.getElapsedTime();
		}
	private:
		virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const
		{
			states.transform *= getTransform();
			states.texture = texture;
			if (arr.size() != 0)
			{
				if (sf::VertexBuffer::isAvailable())
					target.draw(buff, states);
				else
				{
					if (type == ParticleType::QuadsTriangles)
						target.draw(&arr[0], arr.size(), sf::PrimitiveType::Triangles, states);
					else
						target.draw(&arr[0], arr.size(), static_cast<sf::PrimitiveType>(type), states);
				}

			}
		}
	};
	class VertexObject : public sf::Transformable, public sf::Drawable
	{
		sf::VertexArray arr;
		sf::VertexBuffer buff;
		bool useBuffer;
		const sf::Texture* texture;
	public:

		/// <summary>
		/// Default constructor for the vertex object.
		/// </summary>
		VertexObject()
		{
			texture = nullptr;
			useBuffer = sf::VertexBuffer::isAvailable();
			if (sf::VertexBuffer::isAvailable())
				buff.setUsage(sf::VertexBuffer::Dynamic);
		}

		/// <summary>
		/// Uses faster GPU memory for storing vertices.
		/// </summary>
		/// <param name="state">Whether or not to use GPU memory</param>
		void UseBuffer(bool state)
		{
			useBuffer = state;
		}

		/// <summary>
		/// Checks if GPU memory is used for storing vertices.
		/// </summary>
		/// <returns>True if uses GPU memory</returns>
		const bool& isUsingBuffer() const
		{
			return useBuffer;
		}

		/// <summary>
		/// Returns a pointer to the texture. If none is used, returns a nullptr.
		/// </summary>
		/// <returns>Pointer to the texture used</returns>
		const sf::Texture* getTexture() const
		{
			return texture;
		}

		/// <summary>
		/// Specifies a texture to use. Pass nullptr to
		/// disable textures.
		/// </summary>
		/// <param name="texture">Texture to use</param>
		void setTexture(const sf::Texture* texture)
		{
			this->texture = texture;
		}

		/// <summary>
		/// Copies data to GPU memory if it is used.
		/// </summary>
		void Update()
		{
			if (useBuffer && arr.getVertexCount() > 0 && sf::VertexBuffer::isAvailable())
			{
				if (buff.getVertexCount() != arr.getVertexCount())
					buff.create(arr.getVertexCount());
				buff.update(&arr[0]);
			}
		}

		/// <summary>
		/// Clears all vertex data.
		/// </summary>
		void clear()
		{
			arr.clear();
			if (useBuffer && sf::VertexBuffer::isAvailable())
				buff.create(0);
		}

		/// <summary>
		/// Resizes the vertex array.
		/// </summary>
		/// <param name="vertexCount">Number of vertices</param>
		void resize(unsigned int vertexCount)
		{
			arr.resize(vertexCount);
		}

		/// <summary>
		/// Adds an extra vertex to the end of the array.
		/// </summary>
		/// <param name="vertex">Vertex data to add</param>
		void append(const sf::Vertex& vertex)
		{
			arr.append(vertex);
		}

		/// <summary>
		/// Specifies which primitive type should be used.
		/// Note: Quads are not supported on mobile platforms.
		/// </summary>
		/// <param name="type">Primitive type to use</param>
		void setPrimitiveType(sf::PrimitiveType type)
		{
			arr.setPrimitiveType(type);
			if (useBuffer && sf::VertexBuffer::isAvailable())
				buff.setPrimitiveType(type);
		}

		/// <summary>
		/// Returns primitive type which is used.
		/// </summary>
		/// <returns>Primitive type currently used</returns>
		sf::PrimitiveType getPrimitiveType() const
		{
			return arr.getPrimitiveType();
		}

		/// <summary>
		/// Returns a bounding box of all vertices.
		/// </summary>
		/// <returns>Bounding box</returns>
		sf::FloatRect getBounds() const
		{
			return arr.getBounds();
		}

		/// <summary>
		/// Returns how many vertices are currently in the array.
		/// </summary>
		/// <returns>Number of vertices</returns>
		unsigned int getVertexCount() const
		{
			return arr.getVertexCount();
		}

		/// <summary>
		/// Returns a refrence to a vertex in the array.
		/// </summary>
		/// <returns>Reference to the target vertex</returns>
		/// <param name="index">Index of the vertex</param>
		sf::Vertex& operator[] (unsigned int index)
		{
			return arr[index];
		}

	private:
		virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const
		{
			states.transform *= getTransform();
			states.texture = texture;

			if (useBuffer && sf::VertexBuffer::isAvailable())
				target.draw(buff, states);
			else
				target.draw(arr, states);
		}
	};
	class Camera : public sf::View
	{
		float textureMultiplier = 1;
	public:

		/// <summary>
		/// Default constructor for the camera.
		/// </summary>
		Camera()
		{

		}

		/// <summary>
		/// Default deconstructor for the camera.
		/// </summary>
		~Camera()
		{

		}

		/// <summary>
		/// Specifies what number is multiplied to the final texture render resolution. Used by ZLE internally!
		/// </summary>
		/// <param name="vertex">Vertex data to add</param>
		void setTextureMultiplier(float textureMultiplier)
		{
			this->textureMultiplier = textureMultiplier;
		}

		/// <summary>
		/// Returns what number is multiplied to the final texture render resolution. Used by ZLE internally!
		/// </summary>
		/// <returns>Multiplication number</returns>
		float getTextureMultiplier() const
		{
			return textureMultiplier;
		}
	};
	class PointLight : public sf::Transformable, public sf::Drawable
	{
		const sf::Vector2f points[4] = {sf::Vector2f(-1, -1), sf::Vector2f(1, -1), sf::Vector2f(1, 1), sf::Vector2f(-1, 1)};
		float intensity = 0.6f;
		sf::Color color = sf::Color::White;
		sf::Uint16 resolution = 512U;
		float radius = 50.f;
	public:
		class LightTextures
		{
			std::unordered_map<sf::Uint16, sf::Texture> textures;

			static LightTextures& get()
			{
				static LightTextures t;
				return t;
			}
		public:
			static const sf::Texture& getTexture(const sf::Uint16 resolution)
			{
				if (get().textures.find(resolution) == get().textures.end())
				{
					sf::Image tex;
					tex.create(resolution, resolution, sf::Color::Transparent);
					for (int i = 0; i < resolution; i++)
						for (int j = 0; j < resolution; j++)
						{
							sf::Vector2f center = sf::Vector2f(resolution / 2.f, resolution / 2.f);
							float bright = pow((i + 0.5f - center.x) * (i + 0.5f - center.x) + 
								(j + 0.5f - center.y) * (j + 0.5f - center.y), 1 / (2.f * 2.f)) / pow(resolution / 2.f, 1 / 2.f);
							tex.setPixel(i, j, sf::Color(255, 255, 255, std::max(0.f, 1 - bright) * 255.f));
						}
					get().textures[resolution].loadFromImage(tex);
				}
				return get().textures.at(resolution);
			}
		};
		
		PointLight()
		{

		}

		float getIntensity() const
		{
			return intensity;
		}

		const sf::Color& getColor() const
		{
			return color;
		}

		sf::Uint16 getResolution() const
		{
			return resolution;
		}

		float getRadius() const
		{
			return radius;
		}

		void setIntensity(float intensity)
		{
			this->intensity = intensity;
		}

		void setColor(const sf::Color& color)
		{
			this->color = color;
		}

		void setResolution(sf::Uint16 resolution)
		{
			this->resolution = resolution;
		}

		void setRadius(float radius)
		{
			this->radius = radius;
		}

	private:
		virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const
		{
			states.transform *= getTransform();
			states.texture = &LightTextures::getTexture(resolution);
			sf::Vertex arr[4] = {
				sf::Vertex(points[0] * radius, sf::Color(color.r, color.g, color.b, intensity * 255.f), sf::Vector2f(0, 0)),
				sf::Vertex(points[1] * radius, sf::Color(color.r, color.g, color.b, intensity * 255.f), sf::Vector2f(resolution, 0)),
				sf::Vertex(points[3] * radius, sf::Color(color.r, color.g, color.b, intensity * 255.f), sf::Vector2f(0, resolution)),
				sf::Vertex(points[2] * radius, sf::Color(color.r, color.g, color.b, intensity * 255.f), sf::Vector2f(resolution, resolution))
			};
			target.draw(&arr[0], 4, sf::TriangleStrip, states);
		}
	};
	class DirectionalLight : public sf::Transformable, public sf::Drawable
	{
		const sf::Vector2f points[4] = { sf::Vector2f(0, -0.5), sf::Vector2f(1, -0.5), sf::Vector2f(1, 0.5), sf::Vector2f(0, 0.5) };
		float intensity = 0.6f;
		sf::Color color = sf::Color::White;
		sf::Uint16 resolution = 512U;
		sf::Vector2f size = sf::Vector2f(50, 50);

	public:
		class LightTextures
		{
			std::unordered_map<sf::Uint16, sf::Texture> textures;

			static LightTextures& get()
			{
				static LightTextures t;
				return t;
			}
		public:
			static const sf::Texture& getTexture(const sf::Uint16 resolution)
			{
				if (get().textures.find(resolution) == get().textures.end())
				{
					sf::Image tex;
					tex.create(resolution, 1, sf::Color::Transparent);
					for (int i = 0; i < resolution; i++)
					{
						sf::Vector2f center = sf::Vector2f(0.5, 0.5);
						float bright = pow((i + 0.5f - center.x) * (i + 0.5f - center.x) +
							(0.5f - center.y) * (0.5f - center.y), 1 / (2.f * 2.f)) / pow(resolution, 1 / 2.f);
						tex.setPixel(i, 0, sf::Color(255, 255, 255, std::max(0.f, 1 - bright) * 255.f));
					}
					get().textures[resolution].loadFromImage(tex);
				}
				return get().textures.at(resolution);
			}
		};

		DirectionalLight()
		{

		}

		float getIntensity() const
		{
			return intensity;
		}

		const sf::Color& getColor() const
		{
			return color;
		}

		sf::Uint16 getResolution() const
		{
			return resolution;
		}

		const sf::Vector2f& getSize() const
		{
			return size;
		}

		void setIntensity(float intensity)
		{
			this->intensity = intensity;
		}

		void setColor(const sf::Color& color)
		{
			this->color = color;
		}

		void setResolution(sf::Uint16 resolution)
		{
			this->resolution = resolution;
		}

		void setSize(const sf::Vector2f& size)
		{
			this->size = size;
		}

	private:
		virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const
		{
			states.transform *= getTransform();
			states.texture = &LightTextures::getTexture(resolution);
			sf::Vertex arr[4] = {
				sf::Vertex(sf::Vector2f(points[0].x * size.x, points[0].y * size.y), sf::Color(color.r, color.g, color.b, intensity * 255.f), sf::Vector2f(0, 0)),
				sf::Vertex(sf::Vector2f(points[1].x * size.x, points[1].y * size.y), sf::Color(color.r, color.g, color.b, intensity * 255.f), sf::Vector2f(resolution, 0)),
				sf::Vertex(sf::Vector2f(points[3].x * size.x, points[3].y * size.y), sf::Color(color.r, color.g, color.b, intensity * 255.f), sf::Vector2f(0, 1)),
				sf::Vertex(sf::Vector2f(points[2].x * size.x, points[2].y * size.y), sf::Color(color.r, color.g, color.b, intensity * 255.f), sf::Vector2f(resolution, 1))
			};
			target.draw(&arr[0], 4, sf::TriangleStrip, states);
		}
	};

	struct Hitbox : public sf::Drawable
	{
		struct HitLine
		{
			sf::Vector2f start;
			sf::Vector2f end;
			HitLine() : start(0.f, 0.f), end(0.f, 0.f)
			{

			}
		};
		std::vector<HitLine> lines;
	private:
		virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const
		{
			for (int i = 0; i < lines.size(); i++)
			{
				sf::Vertex v[2] = { sf::Vertex(lines[i].start, sf::Color::Cyan), sf::Vertex(lines[i].end, sf::Color::Cyan) };
				target.draw(v, 2, sf::Lines, states);
			}
		}
	};

	enum class ObjectType
	{
		None,
		TileMap,
		Sprite,
		Texture,
		Shader_F,
		Shader_V,
		ParticleSystem,
		Font,
		Text,
		VertexObject,
		Camera,
		PointLight,
		DirectionalLight
	};
	class Level : public sf::Drawable, public sf::Transformable
	{
	public:
		struct Uniforms
		{
			enum class DataTypes
			{
				Unknown = -1,
				Bool,
				Float,
				Int,
				Vec2,
				Vec3,
				Vec4,
				IVec2,
				IVec3,
				IVec4,
			};
			std::vector<DataTypes> types;
			std::vector<std::string> uniformNames;
			unsigned int byteSize;

			Uniforms() 
				: byteSize(0)
			{

			}

		};
		struct RenderStateObject
		{
		private:
			friend class Level;
			std::unique_ptr<sf::Texture> texture;
			std::unique_ptr<sf::Font> font;
			std::string locationData;
			std::unique_ptr<Uniforms> uniforms;
			ObjectType type;


		public:

			/// <summary>
			/// Default constructor for a render state object.
			/// </summary>
			RenderStateObject()
			{
				type = ObjectType::None;
			}

			~RenderStateObject() {}

			/// <summary>
			/// Allocates memory for a texture.
			/// If this succeeds, from now on a texture is used.
			/// </summary>
			/// <returns>True on success, false on fail</returns>
			bool UseTexture()
			{
				if (texture || uniforms || font)
					return 0;
				texture = std::make_unique<sf::Texture>();
				type = ObjectType::Texture;
				return 1;
			}

			/// <summary>
			/// Allocates memory for a font.
			/// If this succeeds, from now on a font is used.
			/// </summary>
			/// <returns>True on success, false on fail</returns>
			bool UseFont()
			{
				if (texture || uniforms || font)
					return 0;
				font = std::make_unique<sf::Font>();
				type = ObjectType::Font;
				return 1;
			}

			/// <summary>
			/// Allocates memory for uniforms.
			/// This type stores uniform layout data.
			/// </summary>
			/// <returns>True on success, false on fail</returns>
			bool UseUniforms()
			{
				if (texture || uniforms || font)
					return 0;
				uniforms = std::make_unique<Uniforms>();
				return 1;
			}

			/// <summary>
			/// Returns the object type.
			/// </summary>
			/// <returns>Reference to object type</returns>
			const ObjectType& getObjectType() const
			{
				return type;
			}

			/// <summary>
			/// Returns true if object is a shader.
			/// </summary>
			/// <returns>True if shader, false otherwise</returns>
			bool isShader() const
			{
				return type == ObjectType::Shader_F || type == ObjectType::Shader_V;
			}

			/// <summary>
			/// Gets the texture used for this object.
			/// </summary>
			/// <returns>Pointer to the texture used or nullptr if none is used</returns>
			sf::Texture* getTexture() const
			{
				return texture.get();
			}

			/// <summary>
			/// Gets the font used for this object.
			/// </summary>
			/// <returns>Pointer to the font used or nullptr if none is used</returns>
			sf::Font* getFont() const
			{
				return font.get();
			}

			/// <summary>
			/// Gets uniforms if they are used.
			/// </summary>
			/// <returns>Pointer to the uniforms or nullptr if they are not used</returns>
			Uniforms* getUniforms() const
			{
				return uniforms.get();
			}

			/// <summary>
			/// Gets texture/shader location used
			/// </summary>
			/// <returns>Pointer to the texture used or nullptr if none is used</returns>
			const std::string& getLocation() const
			{
				return locationData;
			}

			/// <summary>
			/// The location is different based on what object type
			/// is used. If texture is used, location is the file
			/// texture was loaded from. If uniforms are used,
			/// location is the file the shader was loaded from.
			/// </summary>
			/// <param name="location">String representing location</param>
			void setLocation(const std::string& location)
			{
				this->locationData = location;
			}
		};
		struct Object : public Drawable
		{
		private:
			friend class Level;
			std::unique_ptr<sf::Sprite> sprite;
			std::unique_ptr<TileMap> tilemap;
			std::unique_ptr<ParticleSystem> particleSystem;
			std::unique_ptr<sf::Text> text;
			std::unique_ptr<VertexObject> vObject;
			std::unique_ptr<Camera> camera;
			std::unique_ptr<PointLight> pointLight;
			std::unique_ptr<DirectionalLight> directionalLight;
			std::unique_ptr<Hitbox> hitbox;
			const sf::Texture* texture;
			const sf::Font* font;
			mutable std::unique_ptr<sf::Shader> shader;
			mutable std::vector<unsigned char> uniformFData;
			mutable std::vector<unsigned char> uniformVData;
			ObjectType type;
			sf::Int8 drawingZOrder;
			bool visible = 1;
			sf::BlendMode blendMode;
			RenderStateObject* useVShader;
			RenderStateObject* useFShader;
			sf::Color color;
			
			bool anyUsed()
			{
				return tilemap || sprite || particleSystem || text || vObject || camera || pointLight || directionalLight;
			}
		public:
			/// <summary>
			/// Default constructor for a level object.
			/// </summary>
			Object()
				: drawingZOrder(0)
			{
				sprite = nullptr;
				tilemap = nullptr;
				particleSystem = nullptr;
				text = nullptr;
				useVShader = nullptr;
				useFShader = nullptr;
				texture = nullptr;
				font = nullptr;
				vObject = nullptr;
				camera = nullptr;
				pointLight = nullptr;
				directionalLight = nullptr;
				hitbox = nullptr;
				blendMode = sf::BlendAlpha;
				type = ObjectType::None;
			}

			~Object() {}

			/// <summary>
			/// Allocates memory for a sprite.
			/// If this succeeds, from now on a sprite is used.
			/// </summary>
			/// <returns>True on success, false on fail</returns>
			bool UseSprite()
			{
				if (anyUsed())
					return 0;
				sprite = std::make_unique<sf::Sprite>();
				type = ObjectType::Sprite;
				return 1;
			}

			/// <summary>
			/// Allocates memory for a tilemap.
			/// If this succeeds, from now on a tilemap is used.
			/// </summary>
			/// <returns>True on success, false on fail</returns>
			bool UseTileMap()
			{
				if (anyUsed())
					return 0;
				tilemap = std::make_unique<TileMap>();
				type = ObjectType::TileMap;
				return 1;
			}

			/// <summary>
			/// Allocates memory for a particle system.
			/// If this succeeds, from now on a particle system is used.
			/// </summary>
			/// <returns>True on success, false on fail</returns>
			bool UseParticleSystem()
			{
				if (anyUsed())
					return 0;
				particleSystem = std::make_unique<ParticleSystem>();
				type = ObjectType::ParticleSystem;
				return 1;
			}

			/// <summary>
			/// Allocates memory for a text object.
			/// If this succeeds, from now on a text object is used.
			/// </summary>
			/// <returns>True on success, false on fail</returns>
			bool UseText()
			{
				if (anyUsed())
					return 0;
				text = std::make_unique<sf::Text>();
				type = ObjectType::Text;
				return 1;
			}

			/// <summary>
			/// Allocates memory for a vertex object.
			/// If this succeeds, from now on a vertex object is used.
			/// </summary>
			/// <returns>True on success, false on fail</returns>
			bool UseVertexObject()
			{
				if (anyUsed())
					return 0;
				vObject = std::make_unique<VertexObject>();
				type = ObjectType::VertexObject;
				return 1;
			}

			/// <summary>
			/// Allocates memory for a camera.
			/// If this succeeds, from now on a camera is used.
			/// </summary>
			/// <returns>True on success, false on fail</returns>
			bool UseCamera()
			{
				if (anyUsed())
					return 0;
				camera = std::make_unique<Camera>();
				type = ObjectType::Camera;
				return 1;
			}

			/// <summary>
			/// Allocates memory for a point light.
			/// If this succeeds, from now on a point light is used.
			/// </summary>
			/// <returns>True on success, false on fail</returns>
			bool UsePointLight()
			{
				if (anyUsed())
					return 0;
				pointLight = std::make_unique<PointLight>();
				type = ObjectType::PointLight;
				return 1;
			}

			/// <summary>
			/// Allocates memory for a directional light.
			/// If this succeeds, from now on a directional light is used.
			/// </summary>
			/// <returns>True on success, false on fail</returns>
			bool UseDirectionalLight()
			{
				if (anyUsed())
					return 0;
				directionalLight = std::make_unique<DirectionalLight>();
				type = ObjectType::DirectionalLight;
				return 1;
			}

			/// <summary>
			/// Allocates memory for hitboxes, used for lighting.
			/// </summary>
			void UseHitbox()
			{
				hitbox = std::make_unique<Hitbox>();
			}

			/// <summary>
			/// Deallocates memory for hitboxes.
			/// </summary>
			void DeleteHitbox()
			{
				hitbox.reset();
			}

			/// <summary>
			/// Returns a void pointer to the currently used object
			/// type. If none is used, returns nullptr.
			/// </summary>
			/// <returns>Pointer to the object type in use</returns>
			void* getPointer() const
			{
				if (sprite)
					return sprite.get();
				if (tilemap)
					return tilemap.get();
				if (particleSystem)
					return particleSystem.get();
				if (text)
					return text.get();
				if (vObject)
					return vObject.get();
				if (camera)
					return camera.get();
				if (pointLight)
					return pointLight.get();
				if (directionalLight)
					return directionalLight.get();
				return nullptr;
			}

			/// <summary>
			/// Gets the drawing Z layer.
			/// </summary>
			/// <returns>Const reference to the layer used</returns>
			const sf::Int8& getDrawingZLayer() const
			{
				return drawingZOrder;
			}

			/// <summary>
			/// Checks if this object is drawn to the screen.
			/// </summary>
			/// <returns>Reference to the visibility variable</returns>
			const bool& isVisible() const
			{
				return visible;
			}

			/// <summary>
			/// Returns the object type.
			/// </summary>
			/// <returns>Reference to object type</returns>
			const ObjectType& getObjectType() const
			{
				return type;
			}

			/// <summary>
			/// Gets the blend mode used for this object.
			/// </summary>
			/// <returns>Reference to blend mode</returns>
			const sf::BlendMode& getBlendMode() const
			{
				return blendMode;
			}

			/// <summary>
			/// Gets the texture used for this object.
			/// </summary>
			/// <returns>Pointer to the texture used or nullptr if none is used</returns>
			const sf::Texture* getTexture() const
			{
				return texture;
			}

			/// <summary>
			/// Gets the font used for this object.
			/// </summary>
			/// <returns>Pointer to the font used or nullptr if none is used</returns>
			const sf::Font* getFont() const
			{
				return font;
			}

			/// <summary>
			/// Gets the hitbox used for this object.
			/// </summary>
			/// <returns>Pointer to the hitbox used or nullptr if it isn't initialized</returns>
			Hitbox* getHitbox() const
			{
				return hitbox.get();
			}

			/// <summary>
			/// Gets the transformable of this object
			/// </summary>
			/// <returns>Reference to the transformable</returns>
			Transformable* getTransformable() const
			{
				if (tilemap)
					return tilemap.get();
				if (sprite)
					return sprite.get();
				if (particleSystem)
					return particleSystem.get();
				if (text)
					return text.get();
				if (vObject)
					return vObject.get();
				if (pointLight)
					return pointLight.get();
				if (directionalLight)
					return directionalLight.get();
				return nullptr;
			}

			/// <summary>
			/// Returns a valid sprite pointer if a sprite is used.
			/// </summary>
			/// <returns>Pointer to the sprite</returns>
			sf::Sprite* getSprite() const
			{
				if (!sprite)
					sf::err() << "Object not a sprite!" << std::endl;
				return sprite.get();
			}

			/// <summary>
			/// Returns a valid tilemap pointer if a tilemap is used.
			/// </summary>
			/// <returns>Pointer to the tilemap</returns>
			TileMap* getTileMap() const
			{
				if (!tilemap)
					sf::err() << "Object not a tilemap!" << std::endl;
				return tilemap.get();
			}

			/// <summary>
			/// Returns a valid particle system pointer if it is used.
			/// </summary>
			/// <returns>Pointer to the particle system</returns>
			ParticleSystem* getParticleSystem() const
			{
				if (!particleSystem)
					sf::err() << "Object not a particle system!" << std::endl;
				return particleSystem.get();
			}

			/// <summary>
			/// Returns a valid text object pointer if it is used.
			/// </summary>
			/// <returns>Pointer to the text object</returns>
			sf::Text* getText() const
			{
				if (!text)
					sf::err() << "Object not a text object!" << std::endl;
				return text.get();
			}

			/// <summary>
			/// Returns a valid vertex object pointer if it is used.
			/// </summary>
			/// <returns>Pointer to the vertex object</returns>
			VertexObject* getVertexObject() const
			{
				if (!vObject)
					sf::err() << "Object not a vertex object!" << std::endl;
				return vObject.get();
			}

			/// <summary>
			/// Returns a valid camera pointer if it is used.
			/// </summary>
			/// <returns>Pointer to the camera</returns>
			Camera* getCamera() const
			{
				if (!camera)
					sf::err() << "Object not a camera!" << std::endl;
				return camera.get();
			}

			/// <summary>
			/// Returns a valid point light pointer if it is used.
			/// </summary>
			/// <returns>Pointer to the point light</returns>
			PointLight* getPointLight() const
			{
				if (!pointLight)
					sf::err() << "Object not a point light!" << std::endl;
				return pointLight.get();
			}

			/// <summary>
			/// Returns a valid directional light pointer if it is used.
			/// </summary>
			/// <returns>Pointer to the directional light</returns>
			DirectionalLight* getDirectionalLight() const
			{
				if (!directionalLight)
					sf::err() << "Object not a directional light!" << std::endl;
				return directionalLight.get();
			}

			/// <summary>
			/// Gets the shader pointer. Returns nullptr if it isn't used.
			/// </summary>
			/// <returns>Pointer to the shader</returns>
			sf::Shader* getShader() const
			{
				return shader.get();
			}

			/// <summary>
			/// Gets the color that is multiplied to the object.
			/// </summary>
			/// <returns>Reference to the color</returns>
			const sf::Color& getColor() const
			{
				return color;
			}

			/// <summary>
			/// Sets the drawing layer. In ZLE this is a number between
			/// -128 and 127. The drawing layer specifies in which order
			/// this object will be drawn compared to other objects.
			/// If the drawing layer of this object is higher than some
			/// other object, then that other object will be drawn below
			/// this object.
			/// </summary>
			/// <param name="drawingLayer">Number that represents the drawing layer</param>
			void setDrawingLayer(sf::Int8 drawingLayer)
			{
				drawingZOrder = drawingLayer;
			}

			/// <summary>
			/// Sets the visibility. If this variable is false, this
			/// object will not be drawn to the screen. In ZLE if this
			/// object is a particle system, it will completely stop
			/// updating it which might cause problems with events.
			/// </summary>
			/// <param name="visible">Specifies if this object should be visible</param>
			void setVisible(bool visible)
			{
				this->visible = visible;
			}

			/// <summary>
			/// Sets the blend mode.
			/// </summary>
			/// <param name="blendMode">The blendmode to use</param>
			void setBlendMode(const sf::BlendMode& blendMode)
			{
				this->blendMode = blendMode;
			}

			/// <summary>
			/// Sets the texture.
			/// </summary>
			/// <param name="texture">The texture to use</param>
			void setTexture(const sf::Texture* texture)
			{
				this->texture = texture;
			}

			/// <summary>
			/// Sets the font.
			/// </summary>
			/// <param name="font">The font to use</param>
			void setFont(const sf::Font* font)
			{
				this->font = font;
			}

			/// <summary>
			/// Sets the color that is multiplied with the internal
			/// color of this object.
			/// </summary>
			/// <param name="color">The color to use</param>
			void setColor(const sf::Color& color)
			{
				this->color = color;
			}
		private:
			virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const
			{
				if (!visible)
					return;
				if (camera)
					return;
				states.blendMode = blendMode;
				if (shader)
				{
					states.shader = shader.get();
					if (sprite)
						shader->setUniform("u_mvp", (sf::Glsl::Mat4)target.getView().getTransform().getMatrix());
					else
						shader->setUniform("u_mvp", (sf::Glsl::Mat4)(target.getView().getTransform() * getTransformable()->getTransform()).getMatrix());
				}
				if (sprite)
					target.draw(*sprite, states);
				else if (tilemap)
					target.draw(*tilemap, states);
				else if (particleSystem)
					target.draw(*particleSystem, states);
				else if (text)
					target.draw(*text, states);
				else if (vObject)
					target.draw(*vObject, states);
				else if (pointLight)
					target.draw(*pointLight, states);
				else if (directionalLight)
					target.draw(*directionalLight, states);

			}
		};
	protected:
		template<typename T>
		static void Save(std::string& s, T var)
		{
			for (int i = 0; i < sizeof(T); i++)
				s += (reinterpret_cast<char*>(&var))[i];
		}
		template<typename T>
		static void Save(std::ofstream& s, T var)
		{
			for (int i = 0; i < sizeof(T); i++)
				s << (reinterpret_cast<char*>(&var))[i];
		}
		static void SaveStr(std::ofstream& s, const std::string& var)
		{
			sf::Uint32 size = var.size();
			for (int i = 0; i < sizeof(sf::Uint32); i++)
				s << (reinterpret_cast<char*>(&size))[i];
			s << var;
		}
		template<typename T>
		static void Load(sf::InputStream& input, T& t)
		{
			input.read(&t, sizeof(T));
		}
		static void LoadStr(sf::InputStream& input, std::string& data)
		{
			sf::Uint32 size;
			input.read(&size, sizeof(sf::Uint32));
			data.resize(size);
			input.read(&data[0], size);
		}
		static void LoadLine(sf::InputStream& input, std::string& data)
		{
			char x;
			data.clear();
			while (1)
			{
				if (input.read(&x, 1) == -1)
					break;
				if (x == '\r')
					continue;
				if (x == ' ' || x == '\n')
					break;
				data.push_back(x);
			}
		}
		std::map<std::string, Object> objects;
		std::map<std::string, RenderStateObject> texture_objects;

	public:

		/// <summary>
		/// Default constructor for a ZLE level.
		/// </summary>
		Level() {}

		~Level() {}

		/// <summary>
		/// Get a number of render state objects in this level.
		/// </summary>
		/// <returns>Total number of render state objects</returns>
		const unsigned int getLevelObjectCount() const
		{
			return objects.size();
		}

		/// <summary>
		/// Get a reference to a level object which contains,
		/// a sprite, a tilemap or a particle system. It might also
		/// contain a shader if the object uses it.
		/// </summary>
		/// <param name="name">Name of the render state object to return</param>
		/// <returns>Reference to a render state object</returns>
		Object& getLevelObject(const std::string& name)
		{
			return objects.at(name);
		}

		/// <summary>
		/// Get a reference to a std::map which contains all sprites,
		/// tilemaps, particle systems and shaders
		/// </summary>
		/// <returns>Read only reference to render state objects</returns>
		const std::map<std::string, Object>& getLevelMap() const
		{
			return objects;
		}

		/// <summary>
		/// Get a number of render state objects in this level.
		/// </summary>
		/// <returns>Total number of render state objects</returns>
		const unsigned int getRenderStateObjectCount() const
		{
			return texture_objects.size();
		}

		/// <summary>
		/// Get a reference to a render state object which contains a texture
		/// or shader uniforms.
		/// </summary>
		/// <param name="name">Name of the render state object to return</param>
		/// <returns>Reference to a render state object</returns>
		RenderStateObject& getRenderStateObject(const std::string& name)
		{
			return texture_objects.at(name);
		}

		/// <summary>
		/// Get a reference to a std::map which contains all textures
		/// and shaders.
		/// </summary>
		/// <returns>Read only reference to render state objects</returns>
		const std::map<std::string, RenderStateObject>& getRenderStateMap() const
		{
			return texture_objects;
		}

		/// <summary>
		/// Loads a ZLE level from file. Specify where assets are located
		/// with the optional assetFolder parameter. By default, all
		/// assets are assumed to be in the same directory where
		/// the executable is.
		/// </summary>
		/// <param name="fileName">File to load the level from</param>
		/// <param name="assetFolder">Path to the assets</param>
		/// <returns>True on success, false otherwise</returns>
		bool loadFromFile(const std::filesystem::path& fileName, const std::filesystem::path& assetFolder = "")
		{
			sf::FileInputStream stream;
			if (stream.open(fileName.string()))
				return loadFromStream(stream, assetFolder);
			return 0;
		}

		/// <summary>
		/// Loads a ZLE level from stream. Specify where assets are located
		/// with the optional assetFolder parameter. By default, all
		/// assets are assumed to be in the same directory where
		/// the executable is.
		/// </summary>
		/// <param name="stream">Stream to load the level from</param>
		/// <param name="assetFolder">Path to the assets</param>
		/// <returns>True on success, false otherwise</returns>
		bool loadFromStream(sf::InputStream& stream, const std::filesystem::path& assetFolder = "")
		{
			sf::BlendMode blendModes[4] = { sf::BlendMode(sf::BlendMode::One, sf::BlendMode::Zero),
			sf::BlendMode(sf::BlendMode::SrcAlpha, sf::BlendMode::One, sf::BlendMode::Add, sf::BlendMode::One, sf::BlendMode::One, sf::BlendMode::Add),
			sf::BlendMode(sf::BlendMode::SrcAlpha, sf::BlendMode::OneMinusSrcAlpha, sf::BlendMode::Add, sf::BlendMode::One, sf::BlendMode::OneMinusSrcAlpha, sf::BlendMode::Add),
			sf::BlendMode(sf::BlendMode::DstColor, sf::BlendMode::Zero) };

			std::string line;
			LoadStr(stream, line);
			if (line != ZLE_VERSION) //1.6 loader is NOT compatible with 1.4 & 1.5
				return 0;
			int counter;
			int temp;
			float tempF;
			bool tempB;
			Load(stream, counter);
			std::vector<sf::Texture*> textureP;
			std::vector<sf::Font*> fontP;
			std::vector<RenderStateObject*> shaderP;
			textureP.resize(counter);
			fontP.resize(counter);
			shaderP.resize(counter);
			for (int i = 0; i < counter; i++)
			{
				ObjectType otmp;
				Load(stream, otmp);

				LoadStr(stream, line);
				if (otmp != ObjectType::Texture && otmp != ObjectType::Shader_V && otmp != ObjectType::Shader_F && otmp != ObjectType::Font)
				{
					Load(stream, tempB);
					Load(stream, tempB);
					continue;
				}
				texture_objects[line].type = otmp;
				if (texture_objects[line].type == ObjectType::Texture)
					texture_objects[line].UseTexture();
				if (texture_objects[line].type == ObjectType::Font)
					texture_objects[line].UseFont();

				texture_objects[line].locationData = line;

				if (texture_objects[line].type == ObjectType::Texture)
					texture_objects[line].texture->loadFromFile((assetFolder / line).string());
				if (texture_objects[line].type == ObjectType::Font)
					texture_objects[line].font->loadFromFile((assetFolder / line).string());


				Load(stream, tempB);
				if (texture_objects[line].type == ObjectType::Texture)
					texture_objects[line].texture->setSmooth(tempB);
				Load(stream, tempB);
				if (texture_objects[line].type == ObjectType::Texture)
					texture_objects[line].texture->setRepeated(tempB);
				if (texture_objects[line].type != ObjectType::Texture)
				{
					texture_objects[line].UseUniforms();
				}
				textureP[i] = texture_objects[line].texture.get();
				shaderP[i] = &texture_objects[line];
				fontP[i] = texture_objects[line].font.get();

			}
			Load(stream, counter);
			for (int i = 0; i < counter; i++)
			{
				LoadStr(stream, line);

				Object& o = objects[line];
				Load(stream, o.color);
				Load(stream, o.type);
				switch (o.type)
				{
				case ObjectType::Sprite: o.UseSprite(); break;
				case ObjectType::TileMap: o.UseTileMap(); break;
				case ObjectType::ParticleSystem: o.UseParticleSystem(); break;
				case ObjectType::Text: o.UseText(); break;
				case ObjectType::VertexObject: o.UseVertexObject(); break;
				case ObjectType::Camera: o.UseCamera(); break;
				case ObjectType::PointLight: o.UsePointLight(); break;
				case ObjectType::DirectionalLight: o.UseDirectionalLight(); break;
				}
				Load(stream, temp);
				if (temp != -1)
				{
					if (o.type == ObjectType::Text)
						o.setFont(fontP[temp]);
					else
						o.setTexture(textureP[temp]);
				}

				Load(stream, temp);
				if (temp != -1)
				{
					o.useFShader = shaderP[temp];
					int size;
					Load(stream, size);
					char temp2;
					o.uniformFData.resize(size);
					for (int i = 0; i < size; i++)
					{
						Load(stream, temp2);
						o.uniformFData[i] = temp2;
					}
				}

				Load(stream, temp);
				if (temp != -1)
				{
					o.useVShader = shaderP[temp];
					int size;
					Load(stream, size);
					char temp2;
					o.uniformVData.resize(size);
					for (int i = 0; i < size; i++)
					{
						Load(stream, temp2);
						o.uniformVData[i] = temp2;
					}
				}
				if (o.useVShader || o.useFShader)
					o.shader = std::make_unique<sf::Shader>();
				if (o.useVShader && o.useFShader)
					o.shader->loadFromFile((assetFolder / o.useVShader->locationData).string(), (assetFolder / o.useFShader->locationData).string());
				else if (o.useFShader && !o.useVShader)
					o.shader->loadFromFile((assetFolder / o.useFShader->locationData).string(), sf::Shader::Fragment);
				else if (!o.useFShader && o.useVShader)
					o.shader->loadFromFile((assetFolder / o.useVShader->locationData).string(), sf::Shader::Vertex);

				Load(stream, temp);
				o.visible = temp;
				Load(stream, temp);
				o.setDrawingLayer(temp);
				Load(stream, temp);
				o.setBlendMode(blendModes[temp]);

				sf::Vector2f tempV;
				sf::Transformable* t = o.getTransformable();
				if (t)
				{
					Load(stream, tempV);
					t->setPosition(tempV);

					Load(stream, tempF);
					t->setRotation(tempF);

					Load(stream, tempV);
					t->setScale(tempV);

					Load(stream, tempV);
					t->setOrigin(tempV);
				}
				Load(stream, tempB);
				if (tempB)
				{
					Load(stream, temp);
					o.UseHitbox();
					o.getHitbox()->lines.resize(temp);
					for (int i = 0; i < temp; i++)
					{
						Load(stream, tempV);
						o.getHitbox()->lines[i].start = tempV;
						Load(stream, tempV);
						o.getHitbox()->lines[i].end = tempV;
					}
				}

				switch (o.type)
				{
				case ObjectType::Sprite:
				{
					sf::Sprite* s = o.sprite.get();
					s->setColor(o.color);

					sf::IntRect tempI;
					Load(stream, tempI);
					s->setTextureRect(tempI);
					if (o.getTexture())
						s->setTexture(*o.getTexture());
				}
				break;
				case ObjectType::TileMap:
				{
					TileMap* s = o.tilemap.get();
					unsigned short tempSU1, tempSU2;
					sf::Vector2u tempVU;

					Load(stream, tempSU1);
					Load(stream, tempSU2);
					s->setTextureGap(tempSU1, tempSU2);

					Load(stream, tempVU);
					s->setTextureTileSize(tempVU);
					Load(stream, tempVU);
					s->setTileCount(tempVU);
					Load(stream, tempVU);
					s->setTileSize(tempVU);

					s->setColor(o.color);
					sf::Uint16 tileID;
					for (int i = 0; i < s->getTileCount().x; i++)
						for (int j = 0; j < s->getTileCount().y; j++)
						{
							Load(stream, tileID);
							s->setTileID(sf::Vector2u(i, j), tileID);
						}
					s->setTexture(o.getTexture());
				}
				break;
				case ObjectType::ParticleSystem:
				{
					o.particleSystem->loadFromStream(stream, true);
					o.particleSystem->setTexture(o.getTexture());
				}
				break;
				case ObjectType::Text:
				{
					unsigned char style = 0;
					unsigned int size = 0;
					float spacing = 0;
					LoadStr(stream, line);
					Load(stream, size);
					Load(stream, style);
					o.text->setString(line);
					o.text->setCharacterSize(size);
					o.text->setStyle(style);
					Load(stream, spacing);
					o.text->setLineSpacing(spacing);
					Load(stream, spacing);
					o.text->setLetterSpacing(spacing);
					o.text->setFont(*o.getFont());
				}
				break;
				case ObjectType::VertexObject:
				{
					sf::PrimitiveType type;
					bool buffer;
					unsigned int count;
					Load(stream, type);
					Load(stream, buffer);
					Load(stream, count);
					o.vObject->UseBuffer(buffer);
					o.vObject->setPrimitiveType(type);
					o.vObject->resize(count);
					for (int i = 0; i < count; i++)
					{
						sf::Vector2f v;
						sf::Color c;
						Load(stream, v);
						Load(stream, c);
						(*o.vObject)[i].position = v;
						(*o.vObject)[i].color = c;
						Load(stream, v);
						(*o.vObject)[i].texCoords = v;
					}
					if (o.getTexture())
						o.vObject->setTexture(o.getTexture());
					o.vObject->Update();
				}
				break;
				case ObjectType::Camera:
				{
					sf::Vector2f t;
					Load(stream, t);
					o.camera->setCenter(t);
					if (line != "[Main Camera]")
					{
						Load(stream, t);
						o.camera->setSize(t);
						Load(stream, t.x);
						o.camera->setRotation(t.x);
					}
					Load(stream, t.x);
					o.camera->setTextureMultiplier(t.x);
				}
				break;
				case ObjectType::PointLight:
				{
					float t;
					sf::Uint16 t2;
					Load(stream, t);
					o.pointLight->setRadius(t);
					Load(stream, t);
					o.pointLight->setIntensity(t);
					Load(stream, t2);
					o.pointLight->setResolution(t2);
				}
				break;
				case ObjectType::DirectionalLight:
				{
					float t;
					sf::Vector2f t3;
					sf::Uint16 t2;
					Load(stream, t3);
					o.directionalLight->setSize(t3);
					Load(stream, t);
					o.directionalLight->setIntensity(t);
					Load(stream, t2);
					o.directionalLight->setResolution(t2);
				}
				break;
				}

			}
			return 1;
		}

		/// <summary>
		/// Removes all objects and textures from the level.
		/// Not commonly used.
		/// </summary>
		void clear()
		{
			objects.clear();
			texture_objects.clear();
		}

	private:
		virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const
		{
			states.transform *= getTransform();
			for (sf::Int16 i = INT8_MIN; i <= INT8_MAX; i++)
			{
				for (auto& n : objects)
				{
					if (!n.second.visible)
						continue;
					if (n.second.drawingZOrder == i)
						target.draw(n.second, states);
				}
			}
		}
	};
}