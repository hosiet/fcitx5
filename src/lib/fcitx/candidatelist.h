/*
 * Copyright (C) 2016~2016 by CSSlayer
 * wengxt@gmail.com
 *
 * This library is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; see the file COPYING. If not,
 * see <http://www.gnu.org/licenses/>.
 */
#ifndef _FCITX_CANDIDATELIST_H_
#define _FCITX_CANDIDATELIST_H_

#include <fcitx-utils/dynamictrackableobject.h>
#include <fcitx-utils/key.h>
#include <fcitx/text.h>

namespace fcitx {

class InputContext;
class CandidateList;
class PageableCandidateList;
class BulkCandidateList;
class ModifiableCandidateList;

class CandidateListPrivate;

enum class CandidateLayoutHint { NotSet, Vertical, Horizontal };

class CandidateWordPrivate;

class FCITXCORE_EXPORT CandidateWord {
public:
    CandidateWord(Text text = {});
    virtual ~CandidateWord();
    const Text &text() const;
    virtual void select(InputContext *inputContext) const = 0;

protected:
    Text &text();

private:
    std::unique_ptr<CandidateWordPrivate> d_ptr;
    FCITX_DECLARE_PRIVATE(CandidateWord);
};

// basic stuff
class FCITXCORE_EXPORT CandidateList : public DynamicTrackableObject {
public:
    CandidateList();
    virtual ~CandidateList();

    virtual const Text &label(int idx) const = 0;
    virtual std::shared_ptr<const CandidateWord> candidate(int idx) const = 0;
    virtual int size() const = 0;
    virtual int cursorIndex() const = 0;
    virtual CandidateLayoutHint layoutHint() const = 0;

    PageableCandidateList *toPageable() const;
    BulkCandidateList *toBulk() const;
    ModifiableCandidateList *toModifiable() const;

    FCITX_DECLARE_SIGNAL(CandidateList, Update, void());

protected:
    void setPageable(PageableCandidateList *list);
    void setBulk(BulkCandidateList *list);
    void setModifiable(ModifiableCandidateList *list);

private:
    std::unique_ptr<CandidateListPrivate> d_ptr;
    FCITX_DECLARE_PRIVATE(CandidateList);
};

// useful for regular input method
class FCITXCORE_EXPORT PageableCandidateList {
public:
    // Need for paging
    virtual bool hasPrev() const = 0;
    virtual bool hasNext() const = 0;
    virtual void prev() = 0;
    virtual void next() = 0;

    virtual bool usedNextBefore() const = 0;

    // Following are optional.
    virtual int totalPages() const { return -1; }
    virtual int currentPage() const { return -1; }
    virtual void setPage(int) {}
};

// useful for virtual keyboard
class FCITXCORE_EXPORT BulkCandidateList {
public:
    virtual const CandidateWord &candidateFromAll(int idx) const = 0;
    virtual int totalSize() const = 0;
};

// useful for module other than input method
class FCITXCORE_EXPORT ModifiableCandidateList : public BulkCandidateList {
public:
    // All index used there are global index
    virtual void insert(int idx, CandidateWord *word) = 0;
    virtual void remove(int idx) = 0;
    virtual void replace(int idx, CandidateWord *word) = 0;
    virtual void move(int from, int to) = 0;

    void append(CandidateWord *word) { insert(totalSize(), word); }
};

class DisplayOnlyCandidateListPrivate;

class FCITXCORE_EXPORT DisplayOnlyCandidateList : public CandidateList {
public:
    DisplayOnlyCandidateList();
    ~DisplayOnlyCandidateList();

    void setContent(std::vector<std::string> content);
    void setContent(std::vector<Text> content);
    void setLayoutHint(CandidateLayoutHint hint);
    void setCursorIndex(int index);

    // CandidateList
    const fcitx::Text &label(int idx) const override;
    std::shared_ptr<const CandidateWord> candidate(int idx) const override;
    int cursorIndex() const override;
    int size() const override;
    CandidateLayoutHint layoutHint() const override;

private:
    std::unique_ptr<DisplayOnlyCandidateListPrivate> d_ptr;
    FCITX_DECLARE_PRIVATE(DisplayOnlyCandidateList);
};

class CommonCandidateListPrivate;

class FCITXCORE_EXPORT CommonCandidateList : public CandidateList,
                                             public PageableCandidateList,
                                             public ModifiableCandidateList {
public:
    CommonCandidateList();
    ~CommonCandidateList();

    void clear();
    void setSelectionKey(const KeyList &keyList);
    void setPageSize(int size);
    void setLayoutHint(CandidateLayoutHint hint);
    void setCursorIndex(int index);

    // CandidateList
    const fcitx::Text &label(int idx) const override;
    std::shared_ptr<const CandidateWord> candidate(int idx) const override;
    int cursorIndex() const override;
    int size() const override;

    // PageableCandidateList
    bool hasPrev() const override;
    bool hasNext() const override;
    void prev() override;
    void next() override;

    bool usedNextBefore() const override;

    int totalPages() const override;
    int currentPage() const override;
    void setPage(int page) override;

    CandidateLayoutHint layoutHint() const override;

    // BulkCandidateList
    const CandidateWord &candidateFromAll(int idx) const override;
    int totalSize() const override;

    // ModifiableCandidateList
    void insert(int idx, CandidateWord *word) override;
    void remove(int idx) override;
    void replace(int idx, CandidateWord *word) override;
    void move(int from, int to) override;

private:
    void fixAfterUpdate();

    std::unique_ptr<CommonCandidateListPrivate> d_ptr;
    FCITX_DECLARE_PRIVATE(CommonCandidateList);
};
}

#endif // _FCITX_CANDIDATELIST_H_
