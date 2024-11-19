#pragma once
#include <Arduino.h>
#include <Preferences.h>
#include <type_traits>

/**
 * @brief Base class for read-only variables
 * @tparam T The type of value to store
 */
template<typename T>
class ReadOnlyVariable {
protected:
    T value;
    bool (*beforeUpdate)(T) = nullptr;    // Called before update with newValue, returns true to allow change
    void (*onUpdate)(T) = nullptr;        // Called after successful update with newValue
    T (*stringConverter)(const String&) = nullptr;  // Allows custom string to T conversion

    // Default converter implementations
    static T defaultStringConverter(const String& str) {
        if constexpr (std::is_same_v<T, int>) {
            return str.toInt();
        }  
        else if constexpr (std::is_same_v<T, float>) {
            return str.toFloat();
        }
        else if constexpr (std::is_same_v<T, double>) {
            return str.toDouble();
        }
        else if constexpr (std::is_same_v<T, long>) {
            return str.toInt();
        }
        else if constexpr (std::is_same_v<T, String>) {
            return str;
        }
        else if constexpr (std::is_same_v<T, bool>) {
            return str.equalsIgnoreCase("true") || str == "1";
        }
        else {
            return T();
        }
    }
    
    /**
     * @brief Attempts to set a new value with validation and notification
     * @param newValue The new value to set
     * @return true if value was changed successfully or value is unchanged:
     *         - Value was different from current
     *         - BeforeUpdate callback allowed change (or no callback set)
     *         - Value was updated and OnUpdate notified
     *         Returns false if:
     *         - BeforeUpdate callback prevented change
     */
    bool setValue(T newValue) {
        if (this->value == newValue) {
            return true;
        }
        bool allowUpdate = true;
        if (beforeUpdate) {
            allowUpdate = beforeUpdate(newValue);
        }
        if (allowUpdate) {
            this->value = newValue;
            if (onUpdate) {
                onUpdate(this->value);
            }
            return true;
        }
        return false;
    }

    /**
     * @brief Attempts to set value from string using configured converter
     * @param strValue String value to convert and set
     * @return true if value was updated, false if unchanged, invalid, or prevented
     */
    bool setValue(const String& strValue) {
        if (stringConverter) {
            return setValue(stringConverter(strValue));
        }
        return setValue(defaultStringConverter(strValue));
    }

public:
    /**
     * @brief Sets the callback function to be called after value updates
     * @param newCallback Function pointer to call after successful updates
     */
    void setOnUpdate(void (*newCallback)(T)) {
        this->onUpdate = newCallback;
    }

    template<typename U>
    void setOnUpdate(U* instance, void (U::*newCallback)(T)) {
        onUpdate = [instance, newCallback](T newValue) {
            (instance->*newCallback)(newValue);
        };
    }

    /**
     * @brief Clears the onUpdate callback
     */
    void clearOnUpdate() {
        this->onUpdate = nullptr;
    }

    /**
     * @brief Sets the callback function to be called before value updates
     * @param newCallback Function pointer to call before updates, returns true to allow change
     */
    void setBeforeUpdate(bool (*newCallback)(T)) {
        this->beforeUpdate = newCallback;
    }

    template<typename U>
    void setBeforeUpdate(U* instance, bool (U::*newCallback)(T)) {
        beforeUpdate = [instance, newCallback](T newValue) {
            return (instance->*newCallback)(newValue);
        };
    }

    /**
     * @brief Clears the beforeUpdate callback
     */
    void clearBeforeUpdate() {
        this->beforeUpdate = nullptr;
    }

    /**
     * @brief Gets the current stored value
     * @return The current value
     */
    T getValue() const { return value; }

    // Allow implicit conversion to T
    operator T() const { return value; }
};
 

 /**
 * @brief A template class for managing a value with update callbacks
 * @tparam T The type of value to store
 */
template<typename T>
class Variable : public ReadOnlyVariable<T> {
public:
    using ReadOnlyVariable<T>::setValue;  // Make setValue functions public

    Variable(T initialValue = T()) : ReadOnlyVariable<T>(initialValue) {}
};