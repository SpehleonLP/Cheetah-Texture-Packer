#ifndef SPRITEMODEL_H
#define SPRITEMODEL_H
#include "enums.hpp"
#include "Support/shared_array.hpp"
#include <glm/vec2.hpp>
#include <QAbstractItemModel>
#include <memory>

struct Document;
class MainWindow;
class QItemSelection;
class GLViewWidget;

class SpriteModel : public QAbstractItemModel
{
typedef QAbstractItemModel super;
public:
	struct Entity
	{
		Entity() :
			object(-1),
			heirarchy(Heirarchy::Root),
			property(MaterialProperty::None),
			numeric(-1),
			subId(-1)
		{
		}

		Entity(void* data)
		{
			assert(sizeof(Entity) == sizeof(data));
			memcpy(this, &data, sizeof(*this));
		}

		void * data() const
		{
			assert(sizeof(Entity) == sizeof(void*));

			void * data;
			memcpy(&data, this, sizeof(*this));
			return data;
		}

		 int8_t          object;
		Heirarchy        heirarchy;
		MaterialProperty property;
		 int8_t          numeric;
		 int8_t          subId;
		 int8_t          padding[3];
	};

	SpriteModel(MainWindow *parent = nullptr);
	virtual ~SpriteModel() = default;

	void Render(GLViewWidget *, QModelIndex const& index);

	bool doubleClicked(QModelIndex const& index);
	void activated(QModelIndex const& index);

	int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;

	QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
    QModelIndex parent(const QModelIndex &child) const;

  // bool hasChildren(const QModelIndex &parent = QModelIndex()) const;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
	Qt::ItemFlags flags(QModelIndex const&) const;

//	bool setHeaderData(int, Qt::Orientation, QVariant const&, int);
	bool setData(QModelIndex const&, QVariant const&, int);

	void onCustomContextMenu(QModelIndex const&, QPoint const&) {}

	template<int num>
	static QString StringFromArray(std::array<float, num> const& array);
	template<int num>
	static bool ArrayFromString(std::array<float, num> & dst, QString const&);

	static QString StringFromVector(shared_array<uint16_t> const& array);
	static bool VectorFromString(shared_array<uint16_t> & dst, QString const&);

	static bool onSelected(QModelIndex const& index);
	glm::vec2 GetItemSize(QModelIndex const& index) const;

private:
friend class MainWindow;
	bool UpdateMaterial(Entity entity, QString const& value);

	MainWindow * window{};
};

template<int num>
inline QString SpriteModel::StringFromArray(std::array<float, num> const& array)
{
	QString r;

	for(int i = 0; i < num; ++i)
	{
		r += QString::number(array[i]) + " ";
	}

	return r;
}

template<int num>
inline bool SpriteModel::ArrayFromString(std::array<float, num> & dst, QString const& string)
{
	std::array<float, num> eax;
	bool okay;

	auto tokens = string.split(' ', Qt::SkipEmptyParts);

	if(tokens.size() != num)
		return false;

	for(uint32_t i = 0; i < num; ++i)
	{
		eax[i] = tokens[i].toFloat(&okay);
		if(!okay) return false;
	}

	memcpy(&dst[0], &eax[0], sizeof(float) * num);
	return true;
}

#endif // SPRITEMODEL_H
