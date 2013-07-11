/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

/* C++11-style, but C++98-usable, "move references" implementation. */

#ifndef mozilla_Move_h_
#define mozilla_Move_h_

namespace mozilla {

/*
 * "Move" References
 *
 * Some types can be copied much more efficiently if we know the original's
 * value need not be preserved --- that is, if we are doing a "move", not a
 * "copy". For example, if we have:
 *
 *   Vector<T> u;
 *   Vector<T> v(u);
 *
 * the constructor for v must apply a copy constructor to each element of u ---
 * taking time linear in the length of u. However, if we know we will not need u
 * any more once v has been initialized, then we could initialize v very
 * efficiently simply by stealing u's dynamically allocated buffer and giving it
 * to v --- a constant-time operation, regardless of the size of u.
 *
 * Moves often appear in container implementations. For example, when we append
 * to a vector, we may need to resize its buffer. This entails moving each of
 * its extant elements from the old, smaller buffer to the new, larger buffer.
 * But once the elements have been migrated, we're just going to throw away the
 * old buffer; we don't care if they still have their values. So if the vector's
 * element type can implement "move" more efficiently than "copy", the vector
 * resizing should by all means use a "move" operation. Hash tables also need to
 * be resized.
 *
 * The details of the optimization, and whether it's worth applying, vary from
 * one type to the next. And while some constructor calls are moves, many really
 * are copies, and can't be optimized this way. So we need:
 *
 * 1) a way for a particular invocation of a copy constructor to say that it's
 *    really a move, and that the value of the original isn't important
 *    afterwards (although it must still be safe to destroy); and
 *
 * 2) a way for a type (like Vector) to announce that it can be moved more
 *    efficiently than it can be copied, and provide an implementation of that
 *    move operation.
 *
 * The Move(T&) function takes a reference to a T, and returns a MoveRef<T>
 * referring to the same value; that's 1). A MoveRef<T> is simply a reference
 * to a T, annotated to say that a copy constructor applied to it may move that
 * T, instead of copying it. Finally, a constructor that accepts an MoveRef<T>
 * should perform a more efficient move, instead of a copy, providing 2).
 *
 * So, where we might define a copy constructor for a class C like this:
 *
 *   C(const C& rhs) { ... copy rhs to this ... }
 *
 * we would declare a move constructor like this:
 *
 *   C(MoveRef<C> rhs) { ... move rhs to this ... }
 *
 * And where we might perform a copy like this:
 *
 *   C c2(c1);
 *
 * we would perform a move like this:
 *
 *   C c2(Move(c1))
 *
 * Note that MoveRef<T> implicitly converts to T&, so you can pass a MoveRef<T>
 * to an ordinary copy constructor for a type that doesn't support a special
 * move constructor, and you'll just get a copy.  This means that templates can
 * use Move whenever they know they won't use the original value any more, even
 * if they're not sure whether the type at hand has a specialized move
 * constructor.  If it doesn't, the MoveRef<T> will just convert to a T&, and
 * the ordinary copy constructor will apply.
 *
 * A class with a move constructor can also provide a move assignment operator,
 * which runs this's destructor, and then applies the move constructor to
 * *this's memory. A typical definition:
 *
 *   C& operator=(MoveRef<C> rhs) {
 *     this->~C();
 *     new(this) C(rhs);
 *     return *this;
 *   }
 *
 * With that in place, one can write move assignments like this:
 *
 *   c2 = Move(c1);
 *
 * This destroys c1, moves c1's value to c2, and leaves c1 in an undefined but
 * destructible state.
 *
 * This header file defines MoveRef and Move in the mozilla namespace.  It's up
 * to individual containers to annotate moves as such, by calling Move; and it's
 * up to individual types to define move constructors.
 *
 * One hint: if you're writing a move constructor where the type has members
 * that should be moved themselves, it's much nicer to write this:
 *
 *   C(MoveRef<C> c) : x(c->x), y(c->y) { }
 *
 * than the equivalent:
 *
 *   C(MoveRef<C> c) { new(&x) X(c->x); new(&y) Y(c->y); }
 *
 * especially since GNU C++ fails to notice that this does indeed initialize x
 * and y, which may matter if they're const.
 */
template<typename T>
class MoveRef
{
    T* pointer;

  public:
    explicit MoveRef(T& t) : pointer(&t) { }
    T& operator*() const { return *pointer; }
    T* operator->() const { return pointer; }
    operator T& () const { return *pointer; }
};

template<typename T>
inline MoveRef<T>
Move(T& t)
{
  return MoveRef<T>(t);
}

template<typename T>
inline MoveRef<T>
Move(const T& t)
{
  return MoveRef<T>(const_cast<T&>(t));
}

/** Swap |t| and |u| using move-construction if possible. */
template<typename T>
inline void
Swap(T& t, T& u)
{
  T tmp(Move(t));
  t = Move(u);
  u = Move(tmp);
}

} // namespace mozilla

#endif // mozilla_Move_h_
