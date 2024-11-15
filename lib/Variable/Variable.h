// lib/Variable/Variable.h
#pragma once

/**
 * @brief A template class for managing a value with update callbacks
 * @tparam T The type of value to store
 */
template<typename T>
class Variable {
private:
    T value;
    bool (*beforeUpdate)(T) = nullptr;    // Called before update with newValue, returns true to allow change
    void (*onUpdate)(T) = nullptr;        // Called after successful update with newValue

public:
    /**
     * @brief Constructs a new Variable with an initial value and optional beforeUpdate callback
     * @param initialValue The initial value to store (default constructed if not provided)
     * @param beforeUpdateCallback Optional callback to validate updates (nullptr if not provided)
     */
    Variable(T initialValue = T(), bool (*beforeUpdateCallback)(T) = nullptr) 
        : value(initialValue), beforeUpdate(beforeUpdateCallback) {}

    /**
     * @brief Gets the current stored value
     * @return The current value
     */
    T getValue() const { return value; }
    
    /**
     * @brief Attempts to set a new value, triggering callbacks if value changes
     * @param newValue The value to set
     * @return true if value was updated, false if unchanged or update prevented
     */
    bool setValue(T newValue) {
        if (value != newValue) {
            bool allowUpdate = true;
            if (beforeUpdate) {
                allowUpdate = beforeUpdate(newValue);
            }
            if (allowUpdate) {
                value = newValue;
                if (onUpdate) {
                    onUpdate(value);
                }
                return true;
            }
        }
        return false;
    }

    /**
     * @brief Sets the callback function to be called after value updates
     * @param newCallback Function pointer to call after successful updates
     */
    void setOnUpdate(void (*newCallback)(T)) {
        onUpdate = newCallback;
    }

    /**
     * @brief Sets the callback function to be called before value updates
     * @param newCallback Function pointer to call before updates, returns true to allow change
     */
    void setBeforeUpdate(bool (*newCallback)(T)) {
        beforeUpdate = newCallback;
    }

    /**
     * @brief Clears the onUpdate callback
     */
    void clearOnUpdate() {
        onUpdate = nullptr;
    }

    /**
     * @brief Clears the beforeUpdate callback
     */
    void clearBeforeUpdate() {
        beforeUpdate = nullptr;
    }
};