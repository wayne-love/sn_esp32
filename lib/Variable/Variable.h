// lib/Variable/Variable.h
#pragma once
#include <Arduino.h>

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
            return T(); // Default construct for unsupported types
        }
    }

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

    /**
     * @brief Sets the string conversion function for this type
     * @param converter Function that converts String to type T
     */
    static void setStringConverter(T (*converter)(const String&)) {
        stringConverter = converter;
    }


};