#pragma once

#include "CodeGenHelpers.h"
#include "Node.h"


namespace NativeJIT
{
    template <typename T>
    class ReturnNode : public Node<T>
    {
    public:
        ReturnNode(ExpressionTree& tree, Node<T>& child);

        //
        // Overrides of Node methods.
        //
        virtual ExpressionTree::Storage<T> CodeGenValue(ExpressionTree& tree) override;
        virtual void CompileAsRoot(ExpressionTree& tree) override;
        virtual unsigned LabelSubtree(bool isLeftChild) override;
        virtual void Print() const override;

    private:
        Node<T>& m_child;
    };


    //*************************************************************************
    //
    // Template definitions for ReturnNode
    //
    //*************************************************************************
    template <typename T>
    ReturnNode<T>::ReturnNode(ExpressionTree& tree,
                              Node& child)
        : Node(tree),
          m_child(child)
    {
        // There's an implicit parent to the return node: the function it's used by.
        IncrementParentCount();
        child.IncrementParentCount();
    }


    template <typename T>
    typename ExpressionTree::Storage<T> ReturnNode<T>::CodeGenValue(ExpressionTree& tree)
    {
        Assert(GetParentCount() == 1,
               "Unexpected parent count for the root node: %u",
               GetParentCount());

        return m_child.CodeGen(tree);
    }


    template <typename T>
    void ReturnNode<T>::CompileAsRoot(ExpressionTree& tree)
    {
        ExpressionTree::Storage<T> s = CodeGen(tree);

        RegisterType resultRegister(0);

        // Move result into register 0 unless already there.
        if (!(s.GetStorageClass() == StorageClass::Direct
              && s.GetDirectRegister().IsSameHardwareRegister(resultRegister)))
        {
            CodeGenHelpers::Emit<OpCode::Mov>(tree.GetCodeGenerator(), resultRegister, s);
        }

        tree.GetCodeGenerator().EmitEpilogue();
    }


    template <typename T>
    unsigned ReturnNode<T>::LabelSubtree(bool /*isLeftChild*/)
    {
        unsigned child = m_child.LabelSubtree(true);

        SetRegisterCount(child);

        // WARNING: GetRegisterCount() may return a different value than passed to SetRegisterCount().
        return GetRegisterCount();
    }


    template <typename T>
    void ReturnNode<T>::Print() const
    {
        PrintCoreProperties("ReturnNode");
    }
}
