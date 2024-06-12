# Abstract Algebra

Abstract algebra is a field that has fascinated me for so long. I find it
beautiful, what general ways people have found to describe recurring patterns.
And to me, that is what abstract algebra is at its core. It is trying to define
frameworks and abstract concepts which can describe a wide range of concrete
things.

Of course, when I say abstract, I do not mean it as "difficult to understand"
or "abstruse". I mean it as "con-concrete" or "theoretical".

Abstract algebra can be divided into multiple fields. Or, maybe more correctly:
there are many fields that utilize abstract algebra. These include Group
theory, Category theory or Type theory, just to name a few.

## Group Theory

Group theory is probably the simplest among these. I do not mean it as in group
theory has the least depth among the three, but that the basics building blocks
of it a surprisingly simple to understand. I actually ended up learning a
significant portion of it by accident while learning Haskell.

In the Haskell world, the names "Semigroup" and "Monoid" are the ones that pop
up the most, but there are many names related. These are both, for example,
less concrete versions of a so called "Group."

Yes, I agree. The naming is kind of silly. Since "Group" is such an everyday
word feels weird to talk about trying to understand the concept what a group
is. Mathematicians simply needed names for these concepts.

### So What is a Group?

I promised it is gonna be an easy concept. But there is gonna be a challenge:
understanding the *general* concept of groups. I will try to help with that by
giving some specific examples first. So here are some groups that you are
already familiar with:

- Natural numbers (0, 1, 2, ...) with addition (+)
- Positive real numbers with multiplication (*)
- Booleans with the xor operation
- Invertible square matrices with matrix multiplication

As you can see, a group is a collection of some values together with an
operation. So, as a first step, for something to form a group, we need to
define a collection of elements we operate on. This can be anything really. It
could be natural numbers, booleans, matrices, or even a set only containing me.
Yes me, the writer of this post.

It also needs to have an operation. A *binary* operation. That is something
that operates on exactly two things.

These two requirements always need to be satisfied for something to be a group.
Of course, there are also other requirements for something to be a group. But
just these two are important enough that it got its own name. If something
satisfies just these two rules, it is called a *Magma*. All groups are magmas,
but not all magmas are groups.

<!-- Credit Wikipedia -->

![Algebraic Structures: Magma to Group](magma-to-group-structures.svg)
