#ifndef LPE_FILTERPRESET_H
#define LPE_FILTERPRESET_H

#include <data/models/base/Persistable.h>
#include <data/models/base/Filterable.h>

class FilterPreset : public Persistable {
public:
    typedef struct {
        GUID guid;
        int data;
        std::string key;
        std::string name;
    } ItemIdentifier;

    FilterPreset(ItemIdentifier id, TYPE type, FilterMode filter, std::vector<FilterPreset*> childs = std::vector<FilterPreset*>());
    explicit FilterPreset(ProjectStateContext* ctx);
    ~FilterPreset();

    ItemIdentifier mId;
    TYPE mType = TYPE::NOTHING;
    FilterMode mFilter = FilterMode::RECALLED;
    std::vector<FilterPreset*> mChilds;

protected:
    void persistHandler(WDL_FastString &str) const override;
    bool initFromChunkHandler(std::string &key, std::vector<const char *> &params) override;
    bool initFromChunkHandler(std::string &key, ProjectStateContext *ctx) override;
private:
    [[nodiscard]] std::string getChunkId() const override;
};

void FilterPreset_AddPreset(std::vector<FilterPreset*>& presets, FilterPreset* newPreset);
std::vector<std::string*> FilterPreset_GetNames(const std::vector<FilterPreset*>& presets);
FilterPreset* FilterPreset_GetFilterByName(const std::vector<FilterPreset*>& presets, std::string* name);
bool FilterPreset_IsEqual(FilterPreset *a, FilterPreset *b);
void FilterPreset_SortChilds(FilterPreset *a);


#endif //LPE_FILTERPRESET_H